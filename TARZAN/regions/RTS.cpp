#include "RTS.h"
#include "TARZAN/utilities/function_utilities.h"

#include <iostream>
#include <ranges>
#include <utility>
#include "absl/container/flat_hash_set.h"

#define RTS_DEBUG


/**
 * @brief Auxiliary function for the forwardReachability function.
 *
 * @param reg the current region to handle.
 * @param toProcess collects regions that must be processed.
 * @param regionsHashMap a map containing already processed regions.
 * @param clocksIndices a map from clock names to their index in the clocks vector.
 * @param invariants the invariants of the original Timed Automaton.
 */
void insertRegionInMapAndToProcess(const region::Region &reg,
                                   std::deque<region::Region> &toProcess,
                                   absl::flat_hash_set<region::Region, region::RegionHash> &regionsHashMap,
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
    absl::flat_hash_set<Region, RegionHash> regionsHashMap{};

    for (const auto &init: getInitialRegions())
    {
        toProcess.push_back(init);
        regionsHashMap.insert(init);
    }

#ifdef RTS_DEBUG

    unsigned long long int totalRegions = 0;

#endif

    // Starting the timer for measuring computation.
    const auto start = std::chrono::high_resolution_clock::now();

    while (!toProcess.empty())
    {
        std::vector<Region> successors{};

        const Region &currentRegion = explorationTechnique == BFS ? toProcess.front() : toProcess.back();
        const int currentRegionLocation = currentRegion.getLocation();

        if (currentRegionLocation == targetLocation)
        {
            // Ending the timer for measuring computation.
            const auto end = std::chrono::high_resolution_clock::now();

#ifdef RTS_DEBUG

            std::cout << "Total number of computed regions: " << totalRegions << std::endl;

#endif

            std::cout << "Goal region is reachable!\n";

            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Function took: " << duration.count() << " microseconds." << std::endl;

            return { currentRegion };
        }

        // Computing immediate delay successor.
        const Region &delaySuccessor = currentRegion.getImmediateDelaySuccessor(maxConstant);

        // Computing discrete successors.
        const std::vector<transition> &transitions = outTransitions[currentRegionLocation];
        const std::vector<Region> &discreteSuccessors = currentRegion.getImmediateDiscreteSuccessors(transitions, clocksIndices, locationsToInt);

#ifdef RTS_DEBUG

        totalRegions += discreteSuccessors.size() + 1;

#endif

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

    std::cout << "Goal region is not reachable!\n";

    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function took: " << duration.count() << " microseconds." << std::endl;

    return {};
}


// TODO: implementare questa meglio
/*std::vector<region::Region> region::RTS::buildRegionGraphBackwards(std::vector<Region> startingRegions) const
{
    std::vector toProcess{ std::move(startingRegions) };
    std::vector result{ startingRegions };

    absl::flat_hash_set<Region, RegionHash> regionsHashMap{};

    // togliere
    unsigned long long int totalregions = 0;

    const auto start = std::chrono::high_resolution_clock::now();
    while (!toProcess.empty())
    {
        std::vector<Region> predecessors{};

        Region currentRegion = toProcess.back();
        toProcess.pop_back();

#ifdef CHECK_INITIAL_IS_REACHED
        // Il codice che vedi qui è servito solo per fare un esempio veloce del flower, dopo va tolto.
        const auto &clockValuation = currentRegion.getClockValuation();

        bool isGoalReached = true;
        for (const auto &[fst, snd]: clockValuation)
            if (fst != 0 || snd == true)
                isGoalReached = false;

        if (currentRegion.getLocation() == 1 && isGoalReached)
        {
            std::cout << "Total number of computed regions: " << totalregions << std::endl;
            std::cout << "MAX CONSTANT IS: " << maxConstant << std::endl;
            std::cout << currentRegion.toString() << std::endl;
            std::cout << "GOAL REGION IS REACHABLE!\n";
            const auto end = std::chrono::high_resolution_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;
            std::cout << "CURRENT REGION: " << currentRegion.toString() << std::endl;
            std::exit(EXIT_FAILURE);
        }
#endif

        std::vector<Region> delayPredecessors = currentRegion.getImmediateDelayPredecessors();

        std::vector<transition> transitions = inTransitions[currentRegion.getLocation()];
        std::vector<Region> discretePredecessors = currentRegion.getImmediateDiscretePredecessors(transitions, clocksIndices, locationsToInt, maxConstant);

        totalregions += delayPredecessors.size() + discretePredecessors.size();

        for (const auto &reg: delayPredecessors)
        {
            if (!regionsHashMap.contains(reg))
            {
                result.push_back(reg);
                regionsHashMap.insert(reg);
                toProcess.push_back(reg);
            }
        }
        for (const auto &reg: discretePredecessors)
        {
            if (!regionsHashMap.contains(reg))
            {
                result.push_back(reg);
                regionsHashMap.insert(reg);
                toProcess.push_back(reg);
            }
        }
    }

    return result;
}*/
