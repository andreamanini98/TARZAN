#include "RTS.h"
#include "TARZAN/utilities/function_utilities.h"

#include <iostream>
#include <ranges>
#include <sstream>
#include <utility>
#include "absl/container/flat_hash_set.h"
#include "TARZAN/utilities/file_utilities.h"

// #define RTS_DEBUG
#define EARLY_EXIT


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


/**
 * @brief Auxiliary function checking whether the reachability objective has been reached.
 *
 * @param currentRegion the current region.
 * @param targetLocation the target locations of the current region.
 * @param goalClockConstraints the goal clock constraints of the current region.
 * @param intVarConstr the constraints to be satisfied for the integer variables of the current region.
 * @param clocksIndices a map from clock names to clock indices.
 * @return true if the reachability objective has been reached, false otherwise.
 */
inline bool checkIfTargetRegionReached(const region::Region &currentRegion,
                                       const int &targetLocation,
                                       const std::vector<timed_automaton::ast::clockConstraint> &goalClockConstraints,
                                       const std::vector<timed_automaton::ast::clockConstraint> &intVarConstr,
                                       const std::unordered_map<std::string, int> &clocksIndices)
{
    if (currentRegion.getLocation() != targetLocation)
        return false;

    // Checking whether constraints on integer variables are satisfied.
    if (!intVarConstr.empty())
    {
        for (const auto &integerValue: intVarConstr)
        {
            if (!integerValue.isSatisfied(currentRegion.getVariables().at(integerValue.clock), false))
                return false;
        }
    }

    // Checking whether constraints on clocks are satisfied.
    if (!goalClockConstraints.empty())
    {
        const auto &cv = currentRegion.getClockValuation();

        for (const auto &clockConstraint: goalClockConstraints)
        {
            // ReSharper disable once CppTooWideScopeInitStatement
            const int clockIdx = clocksIndices.at(clockConstraint.clock);

            if (!clockConstraint.isSatisfied(cv[clockIdx].first, cv[clockIdx].second))
                return false;
        }
    }

    return true;
}


std::vector<region::Region> region::RTS::forwardReachability(const std::vector<timed_automaton::ast::clockConstraint> &intVarConstr,
                                                             const std::vector<timed_automaton::ast::clockConstraint> &goalClockConstraints,
                                                             const int targetLocation,
                                                             const ssee explorationTechnique) const
{
    // Starting the timer for measuring computation.
    const auto start = std::chrono::high_resolution_clock::now();

    // Initializing auxiliary data structures for reachability computation.
    std::deque<Region> toProcess{};
    std::unordered_set<Region, RegionHash> regionsHashMap{};

    for (const auto &init: getInitialRegions())
    {
        toProcess.push_back(init);
        regionsHashMap.insert(init);
    }

    // Boolean used to track whether a region has clocks or not. If not, delay successors must not be computed.
    const bool isThereAnyClock = !clocksIndices.empty();

    unsigned long long int totalRegions = 0;

    while (!toProcess.empty())
    {
        const Region &currentRegion = explorationTechnique == BFS ? toProcess.front() : toProcess.back();
        const int currentRegionLocation = currentRegion.getLocation();

#ifdef RTS_DEBUG

        std::cout << "Current region:\n" << currentRegion.toString() << std::endl;

#endif

        const bool isTargetRegionReached = checkIfTargetRegionReached(currentRegion,
                                                                      targetLocation,
                                                                      goalClockConstraints,
                                                                      intVarConstr,
                                                                      clocksIndices);

        if (isTargetRegionReached)
        {
            // Ending the timer for measuring computation.
            const auto end = std::chrono::high_resolution_clock::now();

            std::cout << "Goal is reachable\n";
            std::cout << "Number of regions: " << totalRegions << std::endl;

            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Total time       : " << duration.count() << " microseconds." << std::endl;

#ifdef EARLY_EXIT

            std::quick_exit(EXIT_SUCCESS);

#endif

            return { currentRegion };
        }

        // Computing immediate delay successor if there is at least one clock in the region and the current location is not urgent.
        const bool isDelayComputable = isThereAnyClock && !urgentLocations.contains(currentRegionLocation);
        const Region delaySuccessor = isDelayComputable ? currentRegion.getImmediateDelaySuccessor(maxConstants) : Region{};

        // Computing discrete successors.
        const std::vector<transition> &transitions = outTransitions[currentRegionLocation];
        const std::vector<Region> &discreteSuccessors = currentRegion.getImmediateDiscreteSuccessors(transitions, clocksIndices, locationsToInt);

        totalRegions += discreteSuccessors.size() + (isDelayComputable ? 1 : 0);

        // Removing the processed region now since we do not need it anymore.
        explorationTechnique == BFS ? toProcess.pop_front() : toProcess.pop_back();

        // We insert the delay successor first and then the discrete successors.
        if (isDelayComputable && !regionsHashMap.contains(delaySuccessor))
            insertRegionInMapAndToProcess(delaySuccessor, toProcess, regionsHashMap, clocksIndices, invariants);

        for (const auto &discreteSuccessor: discreteSuccessors)
            if (!regionsHashMap.contains(discreteSuccessor))
                insertRegionInMapAndToProcess(discreteSuccessor, toProcess, regionsHashMap, clocksIndices, invariants);
    }

    // No target region has been reached if the while loop ends.
    // Ending the timer for measuring computation.
    const auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Goal is not reachable\n";
    std::cout << "Number of regions: " << totalRegions << std::endl;

    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Total time       : " << duration.count() << " microseconds." << std::endl;

    return {};
}


std::vector<region::Region> region::RTS::forwardReachability(const std::vector<timed_automaton::ast::clockConstraint> &intVarOrClockConstr,
                                                             const int targetLocation,
                                                             const ssee explorationTechnique,
                                                             const bool isIntVarConstr) const
{
    if (isIntVarConstr)
        return forwardReachability(intVarOrClockConstr, std::vector<timed_automaton::ast::clockConstraint>{}, targetLocation, explorationTechnique);
    return forwardReachability(std::vector<timed_automaton::ast::clockConstraint>{}, intVarOrClockConstr, targetLocation, explorationTechnique);
}


std::vector<region::Region> region::RTS::forwardReachability(const int targetLocation,
                                                             const ssee explorationTechnique) const
{
    return forwardReachability(std::vector<timed_automaton::ast::clockConstraint>{},
                               std::vector<timed_automaton::ast::clockConstraint>{},
                               targetLocation,
                               explorationTechnique);
}


std::vector<region::Region> region::RTS::backwardReachability(const std::vector<Region> &startingRegions, const ssee explorationTechnique) const
{
    // Starting the timer for measuring computation.
    const auto start = std::chrono::high_resolution_clock::now();

    // Initializing auxiliary data structures for reachability computation.
    std::deque<Region> toProcess{};
    std::unordered_set<Region, RegionHash> regionsHashMap{};

    for (const auto &startReg: startingRegions)
    {
        toProcess.push_back(startReg);
        regionsHashMap.insert(startReg);
    }

    // Boolean used to track whether a region has clocks or not. If not, delay successors must not be computed.
    const bool isThereAnyClock = !clocksIndices.empty();

    unsigned long long int totalRegions = 0;

    while (!toProcess.empty())
    {
        const Region &currentRegion = explorationTechnique == BFS ? toProcess.front() : toProcess.back();
        const int currentRegionLocation = currentRegion.getLocation();

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
            isInitialRegionReached = std::ranges::find(initialLocations, currentRegionLocation) != initialLocations.end();

        if (isInitialRegionReached)
        {
            // Ending the timer for measuring computation.
            const auto end = std::chrono::high_resolution_clock::now();

            std::cout << "Goal is reachable\n";
            std::cout << "Number of regions: " << totalRegions << std::endl;

            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Total time       : " << duration.count() << " microseconds." << std::endl;

#ifdef EARLY_EXIT

            std::quick_exit(EXIT_SUCCESS);

#endif

            return { currentRegion };
        }

        // Computing immediate delay predecessors if there is at least one clock in the region and the current location is not urgent.
        const bool isDelayComputable = isThereAnyClock && !urgentLocations.contains(currentRegionLocation);
        const std::vector<Region> delayPredecessors = isDelayComputable ? currentRegion.getImmediateDelayPredecessors() : std::vector<Region>{};

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

    std::cout << "Goal is not reachable\n";
    std::cout << "Number of regions: " << totalRegions << std::endl;

    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Total time       : " << duration.count() << " microseconds." << std::endl;

    return {};
}


std::string region::RTS::to_string() const
{
    std::ostringstream oss;

    oss << "=== RTS Information ===\n\n";

    // Clock indices.
    oss << "Clocks (" << clocksIndices.size() << "):\n";
    for (const auto &[clockName, index]: clocksIndices)
        oss << "  " << clockName << " -> index " << index << "\n";
    oss << "\n";

    // Locations.
    oss << "Locations (" << locationsToInt.size() << "):\n";
    for (const auto &[locationName, index]: locationsToInt)
        oss << "  " << locationName << " -> " << index << "\n";
    oss << "\n";

    // Max constants.
    oss << "Max Constants (" << maxConstants.size() << "):\n";
    for (size_t i = 0; i < maxConstants.size(); ++i)
        oss << "  Clock " << i << ": " << maxConstants[i] << "\n";
    oss << "\n";

    // Initial locations.
    oss << "Initial Locations (" << initialLocations.size() << "):\n";
    for (const int loc: initialLocations)
        oss << "  " << loc << "\n";
    oss << "\n";

    // Outgoing transitions.
    oss << "Outgoing Transitions (" << outTransitions.size() << " locations):\n";
    for (size_t loc = 0; loc < outTransitions.size(); ++loc)
    {
        if (!outTransitions[loc].empty())
        {
            oss << "  From location " << loc << " (" << outTransitions[loc].size() << " transitions):\n";
            for (const auto &trans: outTransitions[loc])
                oss << "    " << trans.startingLocation << " -> " << trans.targetLocation << "\n";
        }
    }
    oss << "\n";

    // Incoming transitions.
    oss << "Incoming Transitions (" << inTransitions.size() << " locations):\n";
    for (size_t loc = 0; loc < inTransitions.size(); ++loc)
    {
        if (!inTransitions[loc].empty())
        {
            oss << "  To location " << loc << " (" << inTransitions[loc].size() << " transitions):\n";
            for (const auto &trans: inTransitions[loc])
                oss << "    " << trans.startingLocation << " -> " << trans.targetLocation << "\n";
        }
    }
    oss << "\n";

    // Initial regions.
    oss << "Initial Regions (" << initialRegions.size() << "):\n";
    for (size_t i = 0; i < initialRegions.size(); ++i)
        oss << "  Region " << i << ":\n" << initialRegions[i].toString() << "\n";
    oss << "\n";

    // Invariants.
    oss << "Invariants (" << invariants.size() << " locations with invariants):\n";
    for (const auto &[loc, constraints]: invariants)
        oss << "  Location " << loc << " (" << constraints.size() << " constraints)\n";

    return oss.str();
}
