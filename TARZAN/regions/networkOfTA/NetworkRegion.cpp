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
    for (int regIdx_i = 0; regIdx_i < transitionSize; regIdx_i++)
    {
        for (const auto &transition_i: transitions[regIdx_i].get())
        {
            // If the action does not synchronize, we try to compute the discrete successors of the current region.
            if (!transition_i.action.second.has_value())
            {
                // Creating a temporary region used to compute the discrete successor.
                // We set the variables of this region to the variables of the network: in this way, they will be updated thanks to the discrete successor
                // function of regions, and we can later set the region variables to this updated variables map.
                region::Region tmpReg = regions[regIdx_i].clone();
                tmpReg.set_variables(networkVariables);

                // Reuse pre-allocated vector to avoid allocation overhead.
                singleTransition.clear();
                singleTransition.push_back(transition_i);

                // We now compute the discrete successor for the current transition, which will be used to update the regions vector.
                // Since we pass a single transition to getImmediateDiscreteSuccessors(), the resulting vector of discrete successors will contain at most one region.
                const std::vector<region::Region> &discreteSuccessors =
                        tmpReg.getImmediateDiscreteSuccessors(singleTransition, clockIndices[regIdx_i], locationsToInt[regIdx_i]);

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

                    updateNetRegionWithDiscSucc(netReg, discreteSuccessors[0], regIdx_i, transition_i.clocksToReset, clockIndices);

                    // `netReg` is a local variable that is never used after this call, so we move
                    // it into the vector instead of copying it. This transfers ownership of its
                    // internal data directly, avoiding an unnecessary deep copy.
                    res.emplace_back(std::move(netReg));
                }
            }
            else if (transition_i.action.second == OUTACT || transition_i.action.second == INACT)
            {
                // For each remaining region, we must check whether there is an action synchronizing with the one above.
                for (int regIdx_j = regIdx_i + 1; regIdx_j < transitionSize; regIdx_j++)
                {
                    for (const auto &transition_j: transitions[regIdx_j].get())
                    {
                        // If the action j synchronizes, we check whether it matches the other one in the outer loop.
                        if (!transition_j.action.second.has_value()) continue;

                        // For actions to synchronize, they must have the same name and a different synchronization symbol (one ! and the other ?).
                        if (transition_i.action.first != transition_j.action.first || transition_i.action.second == transition_j.action.second) continue;

                        if (transition_j.action.second == BROADCAST_OUTACT || transition_j.action.second == BROADCAST_INACT) continue;
                        
                        std::vector<region::Region> discreteSuccessors_sender{};
                        std::vector<region::Region> discreteSuccessors_receiver{};
                        
                        // Identify sender (!) and receiver (?) without duplicating logic.
                        const bool is_sender = (transition_i.action.second == OUTACT);
                        const int senderIdx = is_sender ? regIdx_i : regIdx_j;
                        const int receiverIdx = is_sender ? regIdx_j : regIdx_i;
                        const auto& senderTrans = is_sender ? transition_i : transition_j;
                        const auto& receiverTrans = is_sender ? transition_j : transition_i;

                        // The transition with the output action must fire first.
                        auto tmpRegSender = regions[senderIdx].clone();
                        tmpRegSender.set_variables(networkVariables);

                        singleTransition.clear();
                        singleTransition.push_back(senderTrans);

                        discreteSuccessors_sender =
                                tmpRegSender.getImmediateDiscreteSuccessors(singleTransition, clockIndices[senderIdx], locationsToInt[senderIdx]);

                        // If a discrete successor has been found, we compute the one corresponding to the transition with an input action.
                        if (!discreteSuccessors_sender.empty())
                        {
                            auto tmpRegReceiver = regions[receiverIdx].clone();
                            tmpRegReceiver.set_variables(discreteSuccessors_sender[0].getVariables());

                            singleTransition.clear();
                            singleTransition.push_back(receiverTrans);

                            discreteSuccessors_receiver =
                                    tmpRegReceiver.getImmediateDiscreteSuccessors(singleTransition, clockIndices[receiverIdx], locationsToInt[receiverIdx]);
                        }

#ifdef NETWORKREGION_DEBUG
                        assert(discreteSuccessors_sender.size() <= 1);
                        assert(discreteSuccessors_receiver.size() <= 1);
                        std::cout << "Computing synchronizing actions. I num. successors: "
                                << discreteSuccessors_sender.size() << ", j num. successors: " << discreteSuccessors_receiver.size() << std::endl;
#endif

                        // Both transitions must ensure a discrete successor is computed.
                        if (!discreteSuccessors_sender.empty() && !discreteSuccessors_receiver.empty())
                        {
                            // Cloning the current network region to keep the changes confined to this copy.
                            NetworkRegion netReg = clone();

                            // The last computed successor has the most recently updated integer variables.
                            netReg.setNetworkVariables(discreteSuccessors_receiver[0].getVariables());

                            updateNetRegionWithDiscSucc(netReg, discreteSuccessors_sender[0], senderIdx, senderTrans.clocksToReset, clockIndices);
                            updateNetRegionWithDiscSucc(netReg, discreteSuccessors_receiver[0], receiverIdx, receiverTrans.clocksToReset, clockIndices);

                            res.emplace_back(std::move(netReg));
                        }
                    
                    }
                }
            }
            else if (transition_i.action.second == BROADCAST_OUTACT)
            {
                // Try to fire the sender; if its guard fails the broadcast is skipped.
                region::Region tmpReg = regions[regIdx_i].clone();
                tmpReg.set_variables(networkVariables);

                singleTransition.clear();
                singleTransition.push_back(transition_i);

                const std::vector<region::Region> senderSuccessors =
                        tmpReg.getImmediateDiscreteSuccessors(singleTransition, clockIndices[regIdx_i], locationsToInt[regIdx_i]);

                if (senderSuccessors.empty()) continue;

                // To collect all available receivers We create a list of groups (i.e. TA)
                // Each group contains all the transitions that the considered TA could use to receive the signal
                std::vector<std::vector<std::tuple<int, region::Region, const transition*>>> allReceiversOptions;

                for (int regIdx_j = 0; regIdx_j < transitionSize; regIdx_j++)
                {
                    // TA cannot receive its own broadcast, so we skip the TA of the sender.
                    if (regIdx_j == regIdx_i) continue;

                    std::vector<std::tuple<int, region::Region, const transition*>> enabledForThisTA;

                    for (const auto& transition_j : transitions[regIdx_j].get())
                    {
                        // Match the channel name of the receiver with the one of the sender and the broadcast symbol ?? for the receiver.
                        if (transition_j.action.first != transition_i.action.first || transition_j.action.second != BROADCAST_INACT) continue;

                        region::Region tmpRegJ = regions[regIdx_j].clone();
                        tmpRegJ.set_variables(senderSuccessors[0].getVariables());

                        singleTransition.clear();
                        singleTransition.push_back(transition_j);

                        auto succJ = tmpRegJ.getImmediateDiscreteSuccessors(singleTransition, clockIndices[regIdx_j], locationsToInt[regIdx_j]);
                        
                        if (!succJ.empty()) {
                            enabledForThisTA.emplace_back(regIdx_j, succJ[0], &transition_j);
                        }

                    }

                    // If this TA has at least one enabled way to receive, add the group.
                    if (!enabledForThisTA.empty()) {
                        allReceiversOptions.push_back(std::move(enabledForThisTA));
                    }
                }

                // If every TA has 1 enabled transition, we get 1 successor for each.
                // If some TA have multiple enabled transition, we need a Cartesian product to cover all the possible fired groups for each.
    
                // We use a lambda function instead of a separate function to avoid passing many parameters that are already in the scope and to keep the code more compact and readable.
                std::function<void(int, networkOfTA::NetworkRegion, const absl::btree_map<std::string, int>)> generatePaths;
                
                generatePaths = [&](int groupIdx, networkOfTA::NetworkRegion currentNetReg, const absl::btree_map<std::string, int> vars) {
                    // If our index has reached the end of the list of receivers, we are done.
                    if (groupIdx == allReceiversOptions.size()) {
                        // We take the "world" (i.e. network region) we’ve built, set the final variables, and shove it into the results vector
                        currentNetReg.setNetworkVariables(vars);
                        res.emplace_back(std::move(currentNetReg));
                        return;
                    }

                    // Try every transition option for the current TA
                    for (const auto& enabledForThisTA : allReceiversOptions[groupIdx]) {

                        const auto& [rIdx, rSucc, rTrans] = enabledForThisTA;
                        
                        networkOfTA::NetworkRegion branchReg = currentNetReg.clone();
                        
                        updateNetRegionWithDiscSucc(branchReg, rSucc, rIdx, rTrans->clocksToReset, clockIndices);
                        
                        // We call generatePaths again, but we increment groupIdx + 1 --> we made a choice for TA_x, now go make a choice for TA_(x+1) with the "world" (i.e. network region) we’ve built so far and the updated variables of this branch.
                        generatePaths(groupIdx + 1, std::move(branchReg), rSucc.getVariables());
                    }
                };

                // Initialize the network region with the sender's discrete successor and then start the recursive generation of paths for each combination of receivers.
                NetworkRegion initialNetReg = clone();
                updateNetRegionWithDiscSucc(initialNetReg, senderSuccessors[0], regIdx_i, transition_i.clocksToReset, clockIndices);

                if (allReceiversOptions.empty()) {
                    initialNetReg.setNetworkVariables(senderSuccessors[0].getVariables());
                    res.emplace_back(std::move(initialNetReg));
                }
                else {
                    generatePaths(0, std::move(initialNetReg), senderSuccessors[0].getVariables());
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