#include "RTS.h"
#include "TARZAN/utilities/function_utilities.h"

#include <iostream>
#include <ranges>
#include <utility>
#include "absl/container/flat_hash_set.h"

// #define RTS_DEBUG


/**
 * @brief Auxiliary function for the forwardReachability and backwardReachability functions.
 *
 * @param reg the current region to handle.
 * @param toProcess collects regions that must be processed.
 * @param regionsHashMap a map containing already processed regions.
 * @param clocksIndices a map from clock names to their index in the clocks vector.
 * @param invariants the invariants of the original Timed Automaton.
 */
inline void insertRegionInMapAndToProcess(const region::Region &reg,
                                          std::deque<region::Region> &toProcess,
                                          std::unordered_set<region::Region, region::RegionHash> &regionsHashMap,
                                          const std::unordered_map<std::string, int> &clocksIndices,
                                          const absl::flat_hash_map<int, std::vector<timed_automaton::ast::clockConstraint>> &invariants)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const int regLocation = reg.getLocation();

    if (invariants.contains(regLocation))
    {
        if (isInvariantSatisfied(invariants.at(regLocation), reg.getClockValuation(), clocksIndices))
        {
            regionsHashMap.insert(reg);
            toProcess.push_back(reg);
        }
    } else
    {
        regionsHashMap.insert(reg);
        toProcess.push_back(reg);
    }
}


std::vector<region::Region> region::RTS::forwardReachability(const int targetLocation, const ssee explorationTechnique) const
{
    // Initializing auxiliary data structures for reachability computation.
    std::deque<Region> toProcess{};
    std::unordered_set<Region, RegionHash> regionsHashMap{};

    for (const auto &init: getInitialRegions())
    {
        toProcess.push_back(init);
        regionsHashMap.insert(init);
    }

    unsigned long long int totalRegions = 0;

    // Starting the timer for measuring computation.
    const auto start = std::chrono::high_resolution_clock::now();

    while (!toProcess.empty())
    {
        const Region &currentRegion = explorationTechnique == BFS ? toProcess.front() : toProcess.back();
        const int currentRegionLocation = currentRegion.getLocation();

#ifdef RTS_DEBUG

        std::cout << "Current region:\n" << currentRegion.toString() << std::endl;

#endif

        if (currentRegionLocation == targetLocation)
        {
            // Ending the timer for measuring computation.
            const auto end = std::chrono::high_resolution_clock::now();

            std::cout << "Total number of computed regions: " << totalRegions << std::endl;
            std::cout << "Goal region is reachable!\n";

            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Function took: " << duration.count() << " microseconds." << std::endl;

            return { currentRegion };
        }

        // Computing immediate delay successor.
        const Region &delaySuccessor = currentRegion.getImmediateDelaySuccessor(maxConstants);

        // Computing discrete successors.
        const std::vector<transition> &transitions = outTransitions[currentRegionLocation];
        const std::vector<Region> &discreteSuccessors = currentRegion.getImmediateDiscreteSuccessors(transitions, clocksIndices, locationsToInt);

        totalRegions += discreteSuccessors.size() + 1;

        // Removing the processed region now since we do not need it anymore.
        explorationTechnique == BFS ? toProcess.pop_front() : toProcess.pop_back();

        // We insert the delay successor first and then the discrete successors.
        if (!regionsHashMap.contains(delaySuccessor))
            insertRegionInMapAndToProcess(delaySuccessor, toProcess, regionsHashMap, clocksIndices, invariants);

        for (const auto &discreteSuccessor: discreteSuccessors)
            if (!regionsHashMap.contains(discreteSuccessor))
                insertRegionInMapAndToProcess(discreteSuccessor, toProcess, regionsHashMap, clocksIndices, invariants);
    }

    // No target region has been reached if the while loop ends.
    // Ending the timer for measuring computation.
    const auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Total number of computed regions: " << totalRegions << std::endl;
    std::cout << "Goal region is not reachable!\n";

    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function took: " << duration.count() << " microseconds." << std::endl;

    return {};
}


std::vector<region::Region> region::RTS::backwardReachability(const std::vector<Region> &startingRegions, const ssee explorationTechnique) const
{
    // Initializing auxiliary data structures for reachability computation.
    std::deque<Region> toProcess{};
    std::unordered_set<Region, RegionHash> regionsHashMap{};

    for (const auto &startReg: startingRegions)
    {
        toProcess.push_back(startReg);
        regionsHashMap.insert(startReg);
    }

    unsigned long long int totalRegions = 0;

    // Starting the timer for measuring computation.
    const auto start = std::chrono::high_resolution_clock::now();

    while (!toProcess.empty())
    {
        const Region &currentRegion = explorationTechnique == BFS ? toProcess.front() : toProcess.back();

#ifdef RTS_DEBUG

        std::cout << "Current region:\n" << currentRegion.toString() << std::endl;

#endif

        // Getting the clock valuation to see if we reached an initial region.
        const auto &clockValuation = currentRegion.getClockValuation();

        // Checking if an initial region has been reached.
        bool isInitialRegionReached = true;
        for (const auto &[fst, snd]: clockValuation)
            if (fst != 0 || snd == true)
            {
                isInitialRegionReached = false;
                break;
            }

        // TODO: ogni volta scansioni un vettore, magari potrebbe essere meglio un set.
        if (isInitialRegionReached)
            isInitialRegionReached = std::ranges::find(initialLocations, currentRegion.getLocation()) != initialLocations.end();

        if (isInitialRegionReached)
        {
            // Ending the timer for measuring computation.
            const auto end = std::chrono::high_resolution_clock::now();

            std::cout << "Total number of computed regions: " << totalRegions << std::endl;
            std::cout << "An initial region is reachable!\n";

            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;

            return { currentRegion };
        }

        // Computing immediate delay predecessors.
        const std::vector<Region> &delayPredecessors = currentRegion.getImmediateDelayPredecessors();

        // Computing discrete predecessors.
        const std::vector<transition> &transitions = inTransitions[currentRegion.getLocation()];
        const std::vector<Region> &discPreds = currentRegion.getImmediateDiscretePredecessors(transitions, clocksIndices, locationsToInt, maxConstants);

        totalRegions += delayPredecessors.size() + discPreds.size();

        // Removing the processed region now since we do not need it anymore.
        explorationTechnique == BFS ? toProcess.pop_front() : toProcess.pop_back();

        // We insert the delay predecessors first and then the discrete predecessors.
        for (const auto &delayPredecessor: delayPredecessors)
            if (!regionsHashMap.contains(delayPredecessor))
                insertRegionInMapAndToProcess(delayPredecessor, toProcess, regionsHashMap, clocksIndices, invariants);

        for (const auto &discretePredecessor: discPreds)
            if (!regionsHashMap.contains(discretePredecessor))
                insertRegionInMapAndToProcess(discretePredecessor, toProcess, regionsHashMap, clocksIndices, invariants);
    }

    // No initial region has been reached if the while loop ends.
    // Ending the timer for measuring computation.
    const auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Total number of computed regions: " << totalRegions << std::endl;
    std::cout << "An initial region is not reachable!\n";

    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function took: " << duration.count() << " microseconds." << std::endl;

    return {};
}
