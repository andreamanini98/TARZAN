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
 * @param isInvariantFree a boolean telling whether there are no invariants at all to be checked.
 */
inline void insertRegionInMapAndToProcess(const networkOfTA::NetworkRegion &reg,
                                          std::deque<networkOfTA::NetworkRegion> &toProcess,
                                          std::unordered_set<networkOfTA::NetworkRegion, networkOfTA::NetworkRegionHash> &regionsHashMap,
                                          const std::vector<std::unordered_map<std::string, int>> &clocksIndices,
                                          const std::vector<absl::flat_hash_map<int, std::vector<timed_automaton::ast::clockConstraint>>> &invariants,
                                          const bool isInvariantFree)
{
    bool isRegionLegal = true;

    if (!isInvariantFree)
    {
        const auto &regRegs = reg.getRegions();

        for (int i = 0; i < static_cast<int>(regRegs.size()); i++)
            if (invariants[i].contains(regRegs[i].getLocation()))
                if (!isInvariantSatisfied(invariants[i].at(regRegs[i].getLocation()), regRegs[i].getClockValuation(), clocksIndices[i]))
                {
                    isRegionLegal = false;
                    break;
                }
    }

    if (isRegionLegal)
    {
        toProcess.push_back(reg);
        regionsHashMap.insert(reg);
    }
}


/**
 * @brief Auxiliary function checking whether the reachability objective has been reached.
 *
 * @param currentRegionRegions the regions of the current network region.
 * @param currentNetworkVariables the variables of the current network region.
 * @param currentTargetLocations the target locations of the current network region (permuted with regions during canonicalization).
 * @param currentGoalClockConstraints the goal clock constraints of the current network region (permuted with regions during canonicalization).
 * @param intVarConstr the constraints to be satisfied for the integer variables of the current network region.
 * @param clocksIndices a map, for each automaton, from clock names to clock indices.
 * @return true if the reachability objective has been reached, false otherwise.
 *
 * @warning clocksIndices is NOT permuted during canonicalization. This is correct because symmetry reduction only groups structurally identical automata that
 *          share the same clock name-to-index mappings. When regions are permuted within a symmetry group, clocksIndices[i] remains valid since all automata
 *          in that group have identical clock mappings.
 */
inline bool checkIfTargetRegionReached(const std::vector<region::Region> &currentRegionRegions,
                                       const std::vector<std::optional<int>> &currentTargetLocations,
                                       const std::vector<std::vector<timed_automaton::ast::clockConstraint>> &currentGoalClockConstraints,
                                       const absl::btree_map<std::string, int> &currentNetworkVariables,
                                       const std::vector<timed_automaton::ast::clockConstraint> &intVarConstr,
                                       const std::vector<std::unordered_map<std::string, int>> &clocksIndices)
{
    // With symmetry reduction, targetLocations has been permuted with regions during canonicalization,
    // so we can directly check each region against its corresponding target location.
    for (int i = 0; i < static_cast<int>(currentTargetLocations.size()); i++)
    {
        if (currentTargetLocations[i].has_value())
        {
            if (currentRegionRegions[i].getLocation() != currentTargetLocations[i])
                return false;
        }
    }

    // Checking whether constraints on integer variables are satisfied.
    if (!intVarConstr.empty())
    {
        for (const auto &integerValue: intVarConstr)
        {
            if (!integerValue.isSatisfied(currentNetworkVariables.at(integerValue.clock), false))
                return false;
        }
    }

    // Checking whether constraints on clocks are satisfied.
    // With symmetry reduction, goalClockConstraints has been permuted with regions during canonicalization,
    // so we can directly check each region against its corresponding clock constraints.
    if (!currentGoalClockConstraints.empty())
    {
        for (int i = 0; i < static_cast<int>(currentGoalClockConstraints.size()); i++)
        {
            if (!currentGoalClockConstraints[i].empty())
            {
                const auto &cv = currentRegionRegions[i].getClockValuation();

                for (const auto &clockConstraint: currentGoalClockConstraints[i])
                {
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const int clockIdx = clocksIndices[i].at(clockConstraint.clock);

                    if (!clockConstraint.isSatisfied(cv[clockIdx].first, cv[clockIdx].second))
                        return false;
                }
            }
        }
    }

    return true;
}


// TODO: i nuovi campi li usi solo se la symmetry reduction è attiva, magari si può ereditare dalla classe RTSNetwork una RTSNetworkSymmetric?
std::vector<networkOfTA::NetworkRegion> networkOfTA::RTSNetwork::forwardReachability(
    const std::vector<timed_automaton::ast::clockConstraint> &intVarConstr,
    const std::vector<std::vector<timed_automaton::ast::clockConstraint>> &goalClockConstraints,
    const std::vector<std::optional<int>> &targetLocs,
    const ssee explorationTechnique) const
{
    // Starting the timer for measuring computation.
    const auto start = std::chrono::high_resolution_clock::now();

    // Initializing auxiliary data structures for reachability computation.
    std::deque<NetworkRegion> toProcess{};
    std::unordered_set<NetworkRegion, NetworkRegionHash> regionsHashMap{};

    const bool useSymmetryReduction = !symmetryGroups.empty() && symmetryReduction;

    // Apply symmetry reduction to initial regions if enabled
    for (const auto &init: getInitialRegions())
    {
        NetworkRegion regionToInsert = init.clone();

        // Setting target locations for network regions.
        regionToInsert.setTargetLocations(targetLocs);

        // Setting goal clock constraints for network regions.
        regionToInsert.setGoalClockConstraints(goalClockConstraints);

        // Apply canonical form if symmetry reduction is enabled.
        if (useSymmetryReduction)
            regionToInsert = regionToInsert.getCanonicalForm(symmetryGroups);

        toProcess.push_back(regionToInsert);
        regionsHashMap.insert(regionToInsert);
    }


    unsigned long long int totalRegions = 0;

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

        const bool isTargetRegionReached = checkIfTargetRegionReached(currentRegionRegions,
                                                                      currentRegion.getTargetLocations(),
                                                                      currentRegion.getGoalClockConstraints(),
                                                                      currentRegion.getNetworkVariables(),
                                                                      intVarConstr,
                                                                      clocksIndices);

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
        // Apply canonical form if symmetry reduction is enabled
        if (isDelayComputable)
        {
            if (useSymmetryReduction)
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const NetworkRegion canonicalDelaySucc = delaySuccessor.getCanonicalForm(symmetryGroups);
                if (!regionsHashMap.contains(canonicalDelaySucc))
                    insertRegionInMapAndToProcess(canonicalDelaySucc, toProcess, regionsHashMap, clocksIndices, invariants, isInvariantFree);
            } else
            {
                if (!regionsHashMap.contains(delaySuccessor))
                    insertRegionInMapAndToProcess(delaySuccessor, toProcess, regionsHashMap, clocksIndices, invariants, isInvariantFree);
            }
        }

        for (const auto &discreteSuccessor: discreteSuccessors)
        {
            if (useSymmetryReduction)
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const NetworkRegion canonicalDiscSucc = discreteSuccessor.getCanonicalForm(symmetryGroups);
                if (!regionsHashMap.contains(canonicalDiscSucc))
                    insertRegionInMapAndToProcess(canonicalDiscSucc, toProcess, regionsHashMap, clocksIndices, invariants, isInvariantFree);
            } else
            {
                if (!regionsHashMap.contains(discreteSuccessor))
                    insertRegionInMapAndToProcess(discreteSuccessor, toProcess, regionsHashMap, clocksIndices, invariants, isInvariantFree);
            }
        }
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


std::vector<networkOfTA::NetworkRegion> networkOfTA::RTSNetwork::forwardReachability(const std::vector<timed_automaton::ast::clockConstraint> &intVarConstr,
                                                                                     const std::vector<std::optional<int>> &targetLocs,
                                                                                     const ssee explorationTechnique) const
{
    return forwardReachability(intVarConstr,
                               std::vector<std::vector<timed_automaton::ast::clockConstraint>>{},
                               targetLocs,
                               explorationTechnique);
}


std::vector<networkOfTA::NetworkRegion> networkOfTA::RTSNetwork::forwardReachability(
    const std::vector<std::vector<timed_automaton::ast::clockConstraint>> &goalClockConstraints,
    const std::vector<std::optional<int>> &targetLocs,
    const ssee explorationTechnique) const
{
    return forwardReachability(std::vector<timed_automaton::ast::clockConstraint>{},
                               goalClockConstraints,
                               targetLocs,
                               explorationTechnique);
}


std::vector<networkOfTA::NetworkRegion> networkOfTA::RTSNetwork::forwardReachability(const std::vector<std::optional<int>> &targetLocs,
                                                                                     const ssee explorationTechnique) const
{
    return forwardReachability(std::vector<timed_automaton::ast::clockConstraint>{},
                               std::vector<std::vector<timed_automaton::ast::clockConstraint>>{},
                               targetLocs,
                               explorationTechnique);
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

    // Display transitions counts.
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

    // Display symmetry groups.
    oss << "  Symmetry groups:\n";
    if (symmetryGroups.empty())
        oss << "    (none)\n";
    else
    {
        for (size_t i = 0; i < symmetryGroups.size(); i++)
        {
            oss << "    Group [" << i << "]: {";
            for (size_t j = 0; j < symmetryGroups[i].size(); j++)
            {
                oss << symmetryGroups[i][j];
                if (j < symmetryGroups[i].size() - 1)
                    oss << ", ";
            }
            oss << "}\n";
        }
    }


    oss << "}\n";
    return oss.str();
}
