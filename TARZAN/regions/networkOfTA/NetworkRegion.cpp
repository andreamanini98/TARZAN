#include "NetworkRegion.h"
#include "TARZAN/utilities/function_utilities.h"

// #define NETWORKREGION_DEBUG


networkOfTA::NetworkRegion networkOfTA::NetworkRegion::getImmediateDelaySuccessor(const std::vector<std::vector<int>> &maxConstants) const
{
    // If all regions in reg are of class U, the same network region should be returned (both isAorC and clockOrdering should be empty).
    NetworkRegion reg = clone();

    if (!isAorC.empty())
    {
        // Map that (if not empty) will be inserted in the clockOrdering deque front, meaning that clocks exiting from the units of these regions have the smallest fractional part.
        absl::btree_map<int, boost::dynamic_bitset<>> newClockOrdering{};

        for (const int regIdx: isAorC)
        {
            region::Region &regionToDelay = reg.regions[regIdx];

            // Used later to check whether some clocks must be inserted into the newClockOrdering map.
            const boost::dynamic_bitset<> originalX0 = regionToDelay.getX0();

            // Updating regions with the newly computed immediate delay successor of the region corresponding to index regIdx.
            regionToDelay = regionToDelay.getImmediateDelaySuccessor(maxConstants[regIdx]);

            // If the resulting region has at least one bounded clock set, we must check which clocks are still bounded.
            // ReSharper disable once CppTooWideScopeInitStatement
            std::deque<boost::dynamic_bitset<>> newBounded = regionToDelay.getBounded();

            if (!newBounded.empty())
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const boost::dynamic_bitset<> boundedClocksThatLeftTheUnit = originalX0 & newBounded.front();

                // If some clocks are still bounded after leaving the unit, they will have the smallest fractional part.
                // The fact that clock sets are partitions ensures that, if all clocks became unbounded, this bitset has only bits set to 0 inside.
                if (boundedClocksThatLeftTheUnit.any())
                    newClockOrdering[regIdx] = boundedClocksThatLeftTheUnit;
            }
        }

        // All regions have been processed, and they do not belong to either class A or C anymore (no region is either of class A or C anymore).
        reg.isAorC.clear();

        // Updating the clockOrdering deque.
        if (!newClockOrdering.empty())
            reg.clockOrdering.emplace_front(newClockOrdering);
    } else
    {
        if (!clockOrdering.empty())
        {
            // Getting information about which regions must be delayed.
            // ReSharper disable once CppTooWideScopeInitStatement
            const absl::btree_map<int, boost::dynamic_bitset<>> &cOrd = reg.clockOrdering.back();

            for (const auto &regIdx: cOrd | std::views::keys)
            {
                region::Region &regionToDelay = reg.regions[regIdx];

                // Updating regions with the newly computed immediate delay successor of the region corresponding to index regIdx.
                regionToDelay = regionToDelay.getImmediateDelaySuccessor(maxConstants[regIdx]);

                // The region now is either of class A or C.
                reg.isAorC.insert(regIdx);
            }

            // Removing cOrd from clockOrdering, since these clocks are now all inside x0 in their respective region.
            reg.clockOrdering.pop_back();
        }
    }

    return reg;
}


/**
 * @brief Updates the given network region netReg with a new discrete successor and handles the isAorC and clockOrdering attributes.
 *
 * @param netReg the network region to update.
 * @param discSuccReg a region (derived from discrete successor computation) that is used in updating the netReg network region.
 * @param regIdx the index of the current region under analysis.
 * @param resetClocks the clocks that are reset over the transition used to compute discSuccReg.
 * @param clockIndices indices of the clocks as they appear in the clocks vector of a Timed Automaton (for index regIdx).
 *
 * @warning The netReg network region will be directly modified, nothing is returned by this function.
 */
inline void updateNetRegionWithDiscSucc(networkOfTA::NetworkRegion &netReg,
                                        const region::Region &discSuccReg,
                                        const int regIdx,
                                        const std::vector<std::string> &resetClocks,
                                        const std::vector<std::unordered_map<std::string, int>> &clockIndices)
{
    const int numOfClocks = discSuccReg.getNumberOfClocks();

    // We update the regions vector with the new discrete successor.
    netReg.setRegionGivenIndex(regIdx, discSuccReg);

    // If no clocks must be reset, we can return without affecting clockOrdering.
    if (resetClocks.empty() || numOfClocks == 0)
        return;

    // We must remove the clocks that have been reset.
    // Setting a bitset storing clocks that must be removed from a clock map.
    boost::dynamic_bitset<> toReset(numOfClocks);
    for (const auto &clock: resetClocks)
        toReset.set(cIdx(numOfClocks, clockIndices[regIdx].at(clock)));

    auto &clockOrdering = netReg.getModifiableClockOrdering();

    // For each clock map, we remove the clocks that have been reset.
    for (auto &clockMap: clockOrdering)
    {
        if (clockMap.contains(regIdx))
        {
            clockMap[regIdx] &= ~toReset;

            // If the clock map does not preserve the order for any clock of region regIdx, we remove the corresponding key.
            if (clockMap[regIdx].none())
                clockMap.erase(regIdx);
        }
    }

    // Removing empty maps to keep the clockOrdering deque consistent.
    clockOrdering.erase(std::ranges::remove_if(clockOrdering, [](const auto &map) { return map.empty(); }).begin(), clockOrdering.end());

    // If at least one clock has been reset, the region now has clocks with zero fractional part.
    // ReSharper disable once CppTooWideScopeInitStatement
    auto &isAorC = netReg.getModifiableIsAOrC();
    if (!resetClocks.empty() && !isAorC.contains(regIdx))
        isAorC.insert(regIdx);
}


std::vector<networkOfTA::NetworkRegion> networkOfTA::NetworkRegion::getImmediateDiscreteSuccessors(
    const std::vector<std::reference_wrapper<const std::vector<transition>>> &transitions,
    const std::vector<std::unordered_map<std::string, int>> &clockIndices,
    const std::vector<std::unordered_map<std::string, int>> &locationsToInt) const
{
    std::vector<NetworkRegion> res;

#ifdef NETWORKREGION_DEBUG

    std::cout << "Computing discrete successors." << std::endl;
    // This assertion must hold even if some Timed Automata do not have an outgoing transition from the current location (see the warning).
    assert(regions.size() == transitions.size());

#endif

    const int transitionSize = static_cast<int>(transitions.size());

    // Pre-allocate single-element vector to avoid repeated allocations.
    std::vector<transition> singleTransition{};
    singleTransition.reserve(1);

    // First, we try if every single transition can fire (the action of the transition must not synchronize, i.e., it has no ? or ! symbol).
    // Recall that in this way only one transition fires at a given time (just like it happens in Uppaal).
    for (int regIdx = 0; regIdx < transitionSize; regIdx++)
    {
        for (const auto &transition: transitions[regIdx].get())
        {
            // If the action does not synchronize, we try to compute the discrete successors of the current region.
            if (!transition.action.second.has_value())
            {
                // Creating a temporary region used to compute the discrete successor.
                // We set the variables of this region to the variables of the network: in this way, they will be updated thanks to the discrete successor
                // function of regions, and we can later set the region variables to this updated variables map.
                region::Region tmpReg = regions[regIdx].clone();
                tmpReg.set_variables(networkVariables);

                // Reuse pre-allocated vector to avoid allocation overhead.
                singleTransition.clear();
                singleTransition.push_back(transition);

                // We now compute the discrete successor for the current transition, which will be used to update the regions vector.
                // Since we pass a single transition to getImmediateDiscreteSuccessors(), the resulting vector of discrete successors will contain at most one region.
                const std::vector<region::Region> &discreteSuccessors =
                        tmpReg.getImmediateDiscreteSuccessors(singleTransition, clockIndices[regIdx], locationsToInt[regIdx]);

#ifdef NETWORKREGION_DEBUG

                assert(discreteSuccessors.size() <= 1);
                std::cout << "Computing non synchronizing actions. The size of discreteSuccessors is: " << discreteSuccessors.size() << std::endl;

#endif

                // If one discrete successor has been computed, we must update the current network region.
                if (!discreteSuccessors.empty())
                {
                    // Cloning the current network region to keep the changes confined to this copy.
                    NetworkRegion netReg = clone();

                    // We now set the integer network variables by taking the successor ones.
                    netReg.setNetworkVariables(discreteSuccessors[0].getVariables());

                    updateNetRegionWithDiscSucc(netReg, discreteSuccessors[0], regIdx, transition.clocksToReset, clockIndices);

                    res.emplace_back(netReg);
                }
            }
        }
    }

    // Next, we check whether every pair of transitions is synchronized and whether it can fire in pairs with the other synchronizing transition.
    for (int regIdx_i = 0; regIdx_i < transitionSize - 1; regIdx_i++)
    {
        for (const auto &transition_i: transitions[regIdx_i].get())
        {
            // If the action i synchronizes, we try to compute the discrete successors with other synchronizing actions.
            if (transition_i.action.second.has_value())
            {
                // Getting the action of transition i.
                // ReSharper disable once CppUseStructuredBinding
                const auto &transAction_i = transition_i.action;

                // For each remaining region, we must check whether there is an action synchronizing with the one above.
                for (int regIdx_j = regIdx_i + 1; regIdx_j < transitionSize; regIdx_j++)
                {
                    for (const auto &transition_j: transitions[regIdx_j].get())
                    {
                        // If the action j synchronizes, we check whether it matches the other one in the outer loop.
                        if (transition_j.action.second.has_value())
                        {
                            // Getting the action of transition j.
                            // ReSharper disable once CppTooWideScopeInitStatement
                            // ReSharper disable once CppUseStructuredBinding
                            const auto &transAction_j = transition_j.action;

                            // For actions to synchronize, they must have the same name and a different synchronization symbol (one ! and the other ?).
                            if (transAction_i.first == transAction_j.first && transAction_i.second != transAction_j.second)
                            {
                                std::vector<region::Region> discreteSuccessors_i{};
                                std::vector<region::Region> discreteSuccessors_j{};

                                // The transition with the output action must fire first.
                                if (transAction_i.second == OUTACT)
                                {
                                    auto tmpReg = regions[regIdx_i].clone();
                                    tmpReg.set_variables(networkVariables);

                                    singleTransition.clear();
                                    singleTransition.push_back(transition_i);

                                    discreteSuccessors_i =
                                            tmpReg.getImmediateDiscreteSuccessors(singleTransition, clockIndices[regIdx_i], locationsToInt[regIdx_i]);

                                    // If a discrete successor has been found, we compute the one corresponding to the transition with an input action.
                                    if (!discreteSuccessors_i.empty())
                                    {
                                        tmpReg = regions[regIdx_j].clone();
                                        tmpReg.set_variables(discreteSuccessors_i[0].getVariables());

                                        singleTransition.clear();
                                        singleTransition.push_back(transition_j);

                                        discreteSuccessors_j =
                                                tmpReg.getImmediateDiscreteSuccessors(singleTransition, clockIndices[regIdx_j], locationsToInt[regIdx_j]);
                                    }
                                } else
                                {
                                    // We do the same but symmetrically.
                                    auto tmpReg = regions[regIdx_j].clone();
                                    tmpReg.set_variables(networkVariables);

                                    singleTransition.clear();
                                    singleTransition.push_back(transition_j);

                                    discreteSuccessors_j =
                                            tmpReg.getImmediateDiscreteSuccessors(singleTransition, clockIndices[regIdx_j], locationsToInt[regIdx_j]);

                                    if (!discreteSuccessors_j.empty())
                                    {
                                        tmpReg = regions[regIdx_i].clone();
                                        tmpReg.set_variables(discreteSuccessors_j[0].getVariables());

                                        singleTransition.clear();
                                        singleTransition.push_back(transition_i);

                                        discreteSuccessors_i =
                                                tmpReg.getImmediateDiscreteSuccessors(singleTransition, clockIndices[regIdx_i], locationsToInt[regIdx_i]);
                                    }
                                }

#ifdef NETWORKREGION_DEBUG

                                assert(discreteSuccessors_i.size() <= 1);
                                assert(discreteSuccessors_j.size() <= 1);
                                std::cout << "Computing synchronizing actions. Size i: "
                                        << discreteSuccessors_i.size() << ", Size j: " << discreteSuccessors_j.size() << std::endl;
#endif

                                // Both transitions must ensure a discrete successor is computed.
                                if (!discreteSuccessors_i.empty() && !discreteSuccessors_j.empty())
                                {
                                    // Cloning the current network region to keep the changes confined to this copy.
                                    NetworkRegion netReg = clone();

                                    if (transAction_i.second == OUTACT)
                                    {
                                        // The last computed successor has the most recently updated integer variables.
                                        netReg.setNetworkVariables(discreteSuccessors_j[0].getVariables());

                                        updateNetRegionWithDiscSucc(netReg, discreteSuccessors_i[0], regIdx_i, transition_i.clocksToReset, clockIndices);
                                        updateNetRegionWithDiscSucc(netReg, discreteSuccessors_j[0], regIdx_j, transition_j.clocksToReset, clockIndices);
                                    } else
                                    {
                                        // The last computed successor has the most recently updated integer variables.
                                        netReg.setNetworkVariables(discreteSuccessors_i[0].getVariables());

                                        updateNetRegionWithDiscSucc(netReg, discreteSuccessors_j[0], regIdx_j, transition_j.clocksToReset, clockIndices);
                                        updateNetRegionWithDiscSucc(netReg, discreteSuccessors_i[0], regIdx_i, transition_i.clocksToReset, clockIndices);
                                    }

                                    res.emplace_back(netReg);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}


networkOfTA::NetworkRegion networkOfTA::NetworkRegion::getCanonicalForm(const std::vector<std::vector<int>> &symmetryGroups) const
{
    // If no symmetry groups, return a copy.
    if (symmetryGroups.empty())
        return clone();

    NetworkRegion canonical = clone();

    // For each group of symmetric processes.
    for (const auto &group: symmetryGroups)
    {
        const int groupSize = static_cast<int>(group.size());

        if (groupSize <= 1)
            continue;

        // Extract regions and their original indices.
        std::vector<std::pair<int, region::Region>> groupRegions;
        groupRegions.reserve(groupSize);
        for (const int idx: group)
            groupRegions.emplace_back(idx, regions[idx]);

        // Sort by region content to get canonical form using Region's operator<.
        std::ranges::sort(groupRegions, [](const auto &a, const auto &b) {
            return a.second < b.second;
        });

        // Build a permutation map: original_idx -> sorted_idx (permutation contains the original index of the sorted regions).
        std::vector<int> permutation(groupSize);
        for (int i = 0; i < groupSize; i++)
            permutation[i] = groupRegions[i].first;

        // Write back regions in canonical order.
        for (int i = 0; i < groupSize; i++)
            canonical.regions[group[i]] = groupRegions[i].second;

        // Update isAorC with the permutation.
        absl::btree_set<int> newIsAorC;
        for (int i = 0; i < groupSize; i++)
        {
            int oldIdx = permutation[i];
            int newIdx = group[i];
            if (isAorC.contains(oldIdx))
                newIsAorC.insert(newIdx);
        }
        // Remove old entries and add new ones for this group.
        for (const int idx: group)
            canonical.isAorC.erase(idx);
        for (const int idx: newIsAorC)
            canonical.isAorC.insert(idx);

        // Update clockOrdering with the permutation.
        for (auto &clockMap: canonical.clockOrdering)
        {
            absl::btree_map<int, boost::dynamic_bitset<>> newClockMap{};
            for (int i = 0; i < groupSize; i++)
            {
                int oldIdx = permutation[i];
                int newIdx = group[i];
                if (clockMap.contains(oldIdx))
                    newClockMap[newIdx] = clockMap[oldIdx];
            }
            // Replace the entries for this symmetry group.
            for (const int idx: group)
                clockMap.erase(idx);
            for (const auto &[idx, bitset]: newClockMap)
                clockMap[idx] = bitset;
        }

        // Update targetLocations indices with the permutation.
        if (!canonical.targetLocations.empty())
        {
            std::vector<std::optional<int>> newTargetLocations(groupSize);
            for (int i = 0; i < groupSize; i++)
            {
                const int oldIdx = permutation[i];
                newTargetLocations[i] = canonical.targetLocations[oldIdx];
            }
            // Replace the entries for this symmetry group.
            for (int i = 0; i < groupSize; i++)
            {
                const int newIdx = group[i];
                canonical.targetLocations[newIdx] = newTargetLocations[i];
            }
        }

        // Update goalClockConstraints indices with the permutation.
        if (!canonical.goalClockConstraints.empty())
        {
            std::vector<std::vector<timed_automaton::ast::clockConstraint>> newGoalClockConstraints(groupSize);
            for (int i = 0; i < groupSize; i++)
            {
                const int oldIdx = permutation[i];
                newGoalClockConstraints[i] = canonical.goalClockConstraints[oldIdx];
            }
            // Replace the entries for this symmetry group.
            for (int i = 0; i < groupSize; i++)
            {
                const int newIdx = group[i];
                canonical.goalClockConstraints[newIdx] = newGoalClockConstraints[i];
            }
        }
    }

    return canonical;
}


std::string networkOfTA::NetworkRegion::toString() const
{
    std::ostringstream oss;

    oss << "NetworkRegion {\n";
    oss << "  Regions (" << regions.size() << "):\n";

    for (size_t i = 0; i < regions.size(); i++)
    {
        // Indent the Region's toString output.
        std::istringstream regionStream(regions[i].toString());
        std::string line;
        oss << "  [" << i << "]:\n";

        while (std::getline(regionStream, line))
            if (!line.empty())
                oss << "    " << line << "\n";
    }

    oss << "  isAorC (" << isAorC.size() << "): [";
    bool first = true;
    for (const auto &idx: isAorC)
    {
        if (!first)
            oss << ", ";
        first = false;
        oss << idx;
    }

    oss << "]\n";

    oss << "  clockOrdering (" << clockOrdering.size() << "):\n";
    int index = 0;
    for (const auto &map: clockOrdering)
    {
        oss << "    [" << index++ << "]: {";
        bool first2 = true;
        for (const auto &[key, bitset]: map)
        {
            if (!first2)
                oss << ", ";
            first2 = false;
            oss << key << " -> " << bitset;
        }
        oss << "}\n";
    }

    bool first2 = true;
    oss << "  networkVariables: [";
    for (const auto &[fst, snd]: networkVariables)
    {
        if (!first2)
            oss << ", ";
        first2 = false;
        oss << fst << " -> " << snd;
    }
    oss << "]\n";

    oss << "}\n";
    return oss.str();
}
