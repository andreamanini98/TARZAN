#include "RTSNetwork.h"

// #define RTSNETWORK_DEBUG


/**
 * @brief Auxiliary function for the forwardReachability function.
 *
 * @param reg the current network region to handle.
 * @param toProcess collects network regions that must be processed.
 * @param regionsHashMap a map containing already processed network regions.
 * @param clocksIndices a vector of maps from clock names to their index in the clocks vector.
 * @param invariants the invariants of the original Timed Automata.
 */
inline void insertRegionInMapAndToProcess(const networkOfTA::NetworkRegion &reg,
                                          std::deque<networkOfTA::NetworkRegion> &toProcess,
                                          std::unordered_set<networkOfTA::NetworkRegion, networkOfTA::NetworkRegionHash> &regionsHashMap,
                                          const std::vector<std::unordered_map<std::string, int>> &clocksIndices,
                                          const std::vector<absl::flat_hash_map<int, std::vector<timed_automaton::ast::clockConstraint>>> &invariants)
{
    bool isRegionLegal = true;
    const auto &regRegs = reg.getRegions();

    for (int i = 0; i < static_cast<int>(regRegs.size()); i++)
        if (invariants[i].contains(regRegs[i].getLocation()))
            if (!isInvariantSatisfied(invariants[i].at(regRegs[i].getLocation()), regRegs[i].getClockValuation(), clocksIndices[i]))
            {
                isRegionLegal = false;
                break;
            }

    if (isRegionLegal)
    {
        toProcess.push_back(reg);
        regionsHashMap.insert(reg);
    }
}


std::vector<networkOfTA::NetworkRegion> networkOfTA::RTSNetwork::forwardReachability(const std::vector<std::optional<int>> &targetLocs,
                                                                                     const ssee explorationTechnique) const
{
    // Initializing auxiliary data structures for reachability computation.
    std::deque<NetworkRegion> toProcess{};
    std::unordered_set<NetworkRegion, NetworkRegionHash> regionsHashMap{};

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
        const NetworkRegion &currentRegion = explorationTechnique == BFS ? toProcess.front() : toProcess.back();

#ifdef RTSNETWORK_DEBUG

        std::cout << "Current region:\n" << currentRegion.toString() << std::endl;

        std::cout << "Locations of the network region:" << std::endl;
        for (const auto &reg: currentRegion.getRegions())
            std::cout << reg.getLocation() << " ";
        std::cout << "\n\n";

#endif

        // Getting the regions of the network region currentRegion.
        const auto &currentRegionRegions = currentRegion.getRegions();

        // Checking if the target region has been reached.
        bool isTargetRegionReached = true;

        for (int i = 0; i < static_cast<int>(targetLocs.size()); i++)
            if (targetLocs[i].has_value())
            {
                if (currentRegionRegions[i].getLocation() != targetLocs[i])
                {
                    isTargetRegionReached = false;
                    break;
                }
            }

        if (isTargetRegionReached)
        {
            // Ending the timer for measuring computation.
            const auto end = std::chrono::high_resolution_clock::now();

            std::cout << "Total number of computed regions: " << totalRegions << std::endl;
            std::cout << "Goal region is reachable!\n";

            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Function took: " << duration.count() << " microseconds." << std::endl;

            return { currentRegion };
        }

        // Computing a network immediate delay successor if no region is in an urgent location.
        const bool isDelayComputable = !std::ranges::any_of(automataWithUrgentLocations, [&](const auto &pair) {
            return pair.second.contains(currentRegionRegions[pair.first].getLocation());
        });

        const NetworkRegion delaySuccessor = isDelayComputable ? currentRegion.getImmediateDelaySuccessor(maxConstants) : NetworkRegion{};

        // Setting up the transitions for the network discrete successor computation.
        std::vector<std::reference_wrapper<const std::vector<transition>>> transitions{};
        transitions.reserve(currentRegionRegions.size());
        for (int i = 0; i < static_cast<int>(currentRegionRegions.size()); i++)
            transitions.emplace_back(std::cref(outTransitions[i][currentRegionRegions[i].getLocation()]));

        // Computing network discrete successors.
        const std::vector<NetworkRegion> discreteSuccessors = currentRegion.getImmediateDiscreteSuccessors(transitions, clocksIndices, locationsToInt);

        totalRegions += discreteSuccessors.size() + (isDelayComputable ? 1 : 0);

        // Removing now since we do not need it anymore.
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

    std::cout << "Total number of computed regions: " << totalRegions << std::endl;
    std::cout << "Goal region is not reachable!\n";

    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function took: " << duration.count() << " microseconds." << std::endl;

    return {};
}


std::string networkOfTA::RTSNetwork::toString() const
{
    std::ostringstream oss;

    oss << "RTSNetwork {\n";

    // Display automata information.
    oss << "  Automata (" << automata.size() << "):\n";
    for (size_t i = 0; i < automata.size(); i++)
        oss << "    [" << i << "]: " << automata[i].name << "\n";

    // Display max constants.
    oss << "  Max Constants (" << maxConstants.size() << "):\n";
    for (size_t i = 0; i < maxConstants.size(); i++)
    {
        oss << "    Automaton [" << i << "] (" << maxConstants[i].size() << " clocks): [";
        for (size_t j = 0; j < maxConstants[i].size(); j++)
        {
            oss << maxConstants[i][j];
            if (j < maxConstants[i].size() - 1)
                oss << ", ";
        }
        oss << "]\n";
    }
    oss << "\n";

    // Display clocks info.
    oss << "  Clocks:\n";
    for (size_t i = 0; i < clocksIndices.size(); i++)
    {
        oss << "    Automaton [" << i << "] (" << clocksIndices[i].size() << " clocks): {";
        bool first = true;
        for (const auto &[clock, index]: clocksIndices[i])
        {
            if (!first)
                oss << ", ";
            first = false;
            oss << clock << " -> " << index;
        }
        oss << "}\n";
    }

    // Display locations.
    oss << "  Locations:\n";
    for (size_t i = 0; i < locationsToInt.size(); i++)
    {
        oss << "    Automaton [" << i << "] (" << locationsToInt[i].size() << " locations): {";
        bool first = true;
        for (const auto &[loc, index]: locationsToInt[i])
        {
            if (!first)
                oss << ", ";
            first = false;
            oss << loc << " -> " << index;
        }
        oss << "}\n";
    }

    // Display initial locations.
    oss << "  Initial Locations:\n";
    for (size_t i = 0; i < initialLocations.size(); i++)
    {
        oss << "    Automaton [" << i << "]: [";
        for (size_t j = 0; j < initialLocations[i].size(); j++)
        {
            oss << initialLocations[i][j];
            if (j < initialLocations[i].size() - 1)
                oss << ", ";
        }
        oss << "]\n";
    }

    // Display initial network regions.
    oss << "  Initial Regions (" << initialRegions.size() << "):\n";
    for (size_t i = 0; i < initialRegions.size(); i++)
    {
        oss << "    [" << i << "]:\n";
        // Indent the NetworkRegion's toString output.
        std::istringstream regionStream(initialRegions[i].toString());
        std::string line;
        while (std::getline(regionStream, line))
            if (!line.empty())
                oss << "      " << line << "\n";
    }

    // Display transitions counts
    oss << "  Transitions:\n";
    for (size_t i = 0; i < outTransitions.size(); i++)
    {
        int totalOut = 0;
        for (const auto &locTransitions: outTransitions[i])
            totalOut += static_cast<int>(locTransitions.size());

        int totalIn = 0;
        for (const auto &locTransitions: inTransitions[i])
            totalIn += static_cast<int>(locTransitions.size());

        oss << "    Automaton [" << i << "]: "
                << outTransitions[i].size() << " locations, "
                << totalOut << " outgoing transitions, "
                << totalIn << " incoming transitions\n";
    }

    oss << "}\n";
    return oss.str();
}
