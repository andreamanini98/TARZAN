#include "Region.h"

#include <sstream>
#include <iostream>
#include "absl/container/btree_map.h"

#include "TARZAN/utilities/function_utilities.h"
#include "TARZAN/utilities/partition_utilities.h"

// #define REGION_DEBUG


std::vector<std::pair<int, bool>> region::Region::getClockValuation() const
{
    const int numOfClocks = getNumberOfClocks();
    std::vector<std::pair<int, bool>> clockValuation(numOfClocks);

    for (int i = 0; i < numOfClocks; i++)
    {
        clockValuation[i].first = h[i];
        clockValuation[i].second = !x0.test(cIdx(numOfClocks, i));
    }

    return clockValuation;
}


region::Region region::Region::getImmediateDelaySuccessor(const std::vector<int> &maxConstants) const
{
    Region reg = clone();
    const int numOfClocks = getNumberOfClocks();

    if (bounded.empty() && x0.none())
        return reg;
    if (x0.any())
    {
        boost::dynamic_bitset<> xTmp(numOfClocks);
        boost::dynamic_bitset<> xOob(numOfClocks);

        for (int i = 0; i < numOfClocks; i++)
        {
            if (reg.x0.test(cIdx(numOfClocks, i)))
            {
                if (reg.h[i] == maxConstants[i])
                    xOob.set(cIdx(numOfClocks, i), true);
                else
                    xTmp.set(cIdx(numOfClocks, i), true);
            }
        }
        reg.x0.reset();

        if (xTmp.any())
            reg.bounded.push_front(xTmp);
        if (xOob.any())
            reg.unbounded.push_front(xOob);
    } else
    {
        const boost::dynamic_bitset<> &lastBoundedSet = reg.bounded.back();

        for (int i = 0; i < numOfClocks; i++)
            if (lastBoundedSet.test(cIdx(numOfClocks, i)))
                reg.h[i]++;

        reg.x0 |= lastBoundedSet;
        reg.bounded.pop_back();
    }

    return reg;
}


std::vector<region::Region> region::Region::getImmediateDelayPredecessors() const
{
    std::vector<Region> res{};
    const int numOfClocks = getNumberOfClocks();

    Region r0 = clone();

    if (bounded.empty() && x0.none())
    {
        r0.x0 |= r0.unbounded.front();
        r0.unbounded.pop_front();
    } else if (x0.any())
    {
        for (int i = 0; i < numOfClocks; i++)
        {
            if (r0.x0.test(cIdx(numOfClocks, i)))
            {
                r0.h[i]--;
                // If the region cannot have a delay predecessor, return an empty std::vector.
                if (r0.h[i] < 0)
                    return res;
            }
        }
        r0.bounded.push_back(r0.x0);
        r0.x0 &= boost::dynamic_bitset<>(numOfClocks);
    } else
    {
        r0.x0 |= r0.bounded.front();
        r0.bounded.pop_front();

        if (!unbounded.empty())
        {
            Region r1 = clone();
            r1.x0 |= r1.unbounded.front();
            r1.unbounded.pop_front();
            res.push_back(r1);

            Region r2 = r1.clone();
            r2.x0 |= r2.bounded.front();
            r2.bounded.pop_front();
            res.push_back(r2);
        }
    }

    res.push_back(r0);
    return res;
}


std::vector<region::Region> region::Region::getImmediateDiscreteSuccessors(const std::vector<transition> &transitions,
                                                                           const std::unordered_map<std::string, int> &clockIndices,
                                                                           const std::unordered_map<std::string, int> &locationsAsIntMap) const
{
    std::vector<Region> res{};

    const std::vector<std::pair<int, bool>> &clockValuation = getClockValuation();
    const int numOfClocks = getNumberOfClocks();

    for (const auto &transition: transitions)
    {
        // The region must hold the current values of integer variables in order for this evaluation to be performed.
        if (transition.isTransitionSatisfied(clockValuation, clockIndices, variables))
        {
            Region reg = clone();
            reg.set_q(locationsAsIntMap.at(transition.targetLocation));

            if (!transition.clocksToReset.empty())
            {
                boost::dynamic_bitset<> resetClocksMask(numOfClocks);

                for (const std::string &resetClock: transition.clocksToReset)
                {
                    const int resetClockIdx = clockIndices.at(resetClock);
                    reg.h[resetClockIdx] = 0;
                    resetClocksMask.set(cIdx(numOfClocks, resetClockIdx));
                }

                reg.x0 |= resetClocksMask;

                resetClocksMask.flip();

                for (auto &clockSet: reg.unbounded)
                    clockSet &= resetClocksMask;
                for (auto &clockSet: reg.bounded)
                    clockSet &= resetClocksMask;

                std::erase_if(reg.unbounded, [](const auto &clockSet) { return clockSet.none(); });
                std::erase_if(reg.bounded, [](const auto &clockSet) { return clockSet.none(); });
            }

            // Evaluating all integer assignments.
            // The region must hold the current values of integer variables in order for this evaluation to be performed.
            for (const auto &intAss: transition.integerAssignments)
                intAss.evaluate(reg.variables);

            res.emplace_back(reg);
        }
    }

    return res;
}


std::vector<region::Region> region::Region::permRegsBounded(const int qReg,
                                                            const std::vector<int> &H,
                                                            const std::deque<boost::dynamic_bitset<>> &unboundedReg,
                                                            boost::dynamic_bitset<> x0Reg,
                                                            const std::deque<boost::dynamic_bitset<>> &boundedReg,
                                                            const int numOfClocks,
                                                            boost::dynamic_bitset<> X,
                                                            const std::vector<int> &maxConstants,
                                                            const boost::dynamic_bitset<> &notInX0,
                                                            const boost::dynamic_bitset<> &notFractionalPart)
{
    std::vector<Region> res{};

    // Checking if clocks of X can be inserted into x0 to ease the partitioning of set X.
    boost::dynamic_bitset<> maxConstantClockMask(numOfClocks);

    for (int i = 0; i < numOfClocks; i++)
        if (X.test(cIdx(numOfClocks, i)) && H[i] == maxConstants[i])
            maxConstantClockMask.set(cIdx(numOfClocks, i));

    x0Reg |= maxConstantClockMask;
    maxConstantClockMask.flip();
    X &= maxConstantClockMask;

    std::vector<std::vector<int>> partitions{};
    std::vector<int> activeBitsIndices{};

    // Getting the partitions as Restricted Growth Strings.
    partitionBitset(X, partitions, activeBitsIndices);

    // Now getting the partitions as vectors of bitsets (surprisingly, separating this task from partitionBitset() is faster).
    const std::vector<std::vector<boost::dynamic_bitset<>>> &bitPart = getBitsetsFromRestrictedGrowthStrings(numOfClocks, partitions, activeBitsIndices);

#ifdef REGION_DEBUG

    std::cout << std::endl;
    std::cout << "REGION_DEBUG: After getBitsetsFromRestrictedGrowthStrings():\n";
    std::cout << "              bitPart.size(): " << bitPart.size() << "\n";
    std::cout << "REGION_DEBUG: Partitions:\n";

    for (size_t idx = 0; idx < bitPart.size(); idx++)
    {
        std::cout << "              [" << idx << "]: [ ";

        const auto &innerVector = bitPart[idx];
        for (size_t jdx = 0; jdx < innerVector.size(); jdx++)
        {
            std::cout << innerVector[jdx];
            if (jdx < innerVector.size() - 1)
                std::cout << ", ";
        }

        std::cout << " ]";
        if (idx < bitPart.size() - 1)
            std::cout << ",";
        std::cout << "\n";
    }

#endif

    // Slots account for the positions in the deque in which to insert clocks.
    // 2 * (bounded.size() + 1) : the +1 accounts for x0, the 2* for the possibility to insert clock sets in between existing ones.
    const int totalSlots = static_cast<int>(2 * (boundedReg.size() + 1));

    // Now processing each partition.
    for (int i = 0; i < static_cast<int>(partitions.size()); i++)
    {
        const std::vector<boost::dynamic_bitset<>> &partition = bitPart[i];
        const int totalElementsInPartition = static_cast<int>(partition.size());

        // Now computing the total ways in which to insert elements in a deque.
        // TODO: vedere se si riesce a fare a meno di calcolare direttamente l'esponenziale.
        const auto total = static_cast<size_t>(std::pow(totalSlots, totalElementsInPartition));

#ifdef REGION_DEBUG

        std::cout << std::endl;
        std::cout << "REGION_DEBUG: totalElementsInPartition: " << totalElementsInPartition << "\n";
        std::cout << "REGION_DEBUG: Partition bitsets:\n";

        for (int jdx = 0; jdx < totalElementsInPartition; jdx++)
            std::cout << "              [" << jdx << "]: " << partition[jdx] << "\n";

#endif

        for (size_t j = 0; j < total; j++)
        {
            // Making these copies so that each newly generated Region can have its own x0 and bounded sets.
            boost::dynamic_bitset<> newX0 = x0Reg;
            std::deque<boost::dynamic_bitset<>> newBounded = boundedReg;

            // Variable used to identify the positions in which to insert partition elements.
            int ctr = static_cast<int>(j);

            // Keeps track of the clock sets which must be inserted last, otherwise the indices of the deque would be modified during the loop.
            absl::btree_map<int, std::vector<boost::dynamic_bitset<>>, std::greater<>> insertionOrder;

            for (int k = 0; k < totalElementsInPartition; k++)
            {
                // The index in which to insert the k-th element of a partition (need to be adjusted, see below).
                const int index = ctr % totalSlots;

                if (index == 0)
                    // Index 0: we must insert the clocks in x0.
                    newX0 |= partition[k];
                else if (index % 2 == 0)
                    // Even index: we must insert clocks in an already existing set (we need to adjust idx since the deque does not contain x0).
                    newBounded.at((index - 1) / 2) |= partition[k];
                else
                    // Odd index: the clock set must be inserted in-between existing ones, but we do it later to avoid modifying the indices during the loop.
                    insertionOrder[index / 2].emplace_back(partition[k]);

#ifdef REGION_DEBUG

                std::cout << std::endl;
                std::cout << "REGION_DEBUG: Processing element " << k << " (bitset: " << partition[k] << ") at slot " << index << "\n";

#endif

                // Advancing the index.
                ctr /= totalSlots;
            }

#ifdef REGION_DEBUG

            std::cout << "REGION_DEBUG: Insertion order map:\n";
            for (auto &[key, vec]: insertionOrder)
            {
                std::cout << "              Key " << key << ":\n";
                for (size_t currentCombination = 0; currentCombination < vec.size(); currentCombination++)
                    std::cout << "                [" << currentCombination << "]: " << vec[currentCombination] << "\n";

                std::cout << "REGION_DEBUG: Now printing permutations:\n";

                std::ranges::sort(vec, [](const auto &a, const auto &b) { return a.to_ulong() < b.to_ulong(); });
                do
                {
                    std::cout << "              ";
                    for (const auto &bs: vec)
                        std::cout << bs << " ";
                    std::cout << "\n";
                } while (std::ranges::next_permutation(vec, [](const auto &a, const auto &b) { return a.to_ulong() < b.to_ulong(); }).found);
            }

#endif

            // Checking if the partition has been correctly inserted, i.e., x0 must not contain any clock belonging to notInx0.
            if ((newX0 & notInX0).none() && (notFractionalPart & newX0) == notFractionalPart)
            {
                // Inserting missing clock sets in the deque.
                const std::vector<std::deque<boost::dynamic_bitset<>>> &permutedDeques = generateAllDeques(insertionOrder, newBounded);

#ifdef REGION_DEBUG

                std::cout << std::endl;
                std::cout << "REGION_DEBUG: permutedDeques (size " << permutedDeques.size() << "):\n";

                for (size_t idx = 0; idx < permutedDeques.size(); idx++)
                {
                    std::cout << "              [" << idx << "]: ";
                    for (const auto &bitset: permutedDeques[idx])
                        std::cout << bitset << " ";
                    std::cout << "\n";
                }

#endif

                // For each obtained deque, we generate a new Region.
                for (const auto &permutedDeque: permutedDeques)
                {
                    // Putting an empty map for variables placeholder, since in discrete predecessors integer variables are not considered.
                    Region reg(qReg, H, unboundedReg, newX0, permutedDeque, {});
                    res.emplace_back(reg);
                }
            }
        }
    }

    return res;
}


std::vector<region::Region> region::Region::permRegsUnbounded(const int qReg,
                                                              const std::vector<int> &H,
                                                              const std::deque<boost::dynamic_bitset<>> &unboundedReg,
                                                              const boost::dynamic_bitset<> &x0Reg,
                                                              const std::deque<boost::dynamic_bitset<>> &boundedReg,
                                                              const int numOfClocks,
                                                              const boost::dynamic_bitset<> &X)
{
    std::vector<Region> res{};

    std::vector<std::vector<int>> partitions{};
    std::vector<int> activeBitsIndices{};

    // Getting the partitions as Restricted Growth Strings.
    partitionBitset(X, partitions, activeBitsIndices);

    // Now getting the partitions as vectors of bitsets (surprisingly, separating this task from partitionBitset() is faster).
    const std::vector<std::vector<boost::dynamic_bitset<>>> &bitPart = getBitsetsFromRestrictedGrowthStrings(numOfClocks, partitions, activeBitsIndices);

#ifdef REGION_DEBUG

    std::cout << std::endl;
    std::cout << "REGION_DEBUG: After getBitsetsFromRestrictedGrowthStrings():\n";
    std::cout << "              bitPart.size(): " << bitPart.size() << "\n";
    std::cout << "REGION_DEBUG: Partitions:\n";

    for (size_t idx = 0; idx < bitPart.size(); idx++)
    {
        std::cout << "              [" << idx << "]: [ ";

        const auto &innerVector = bitPart[idx];
        for (size_t jdx = 0; jdx < innerVector.size(); jdx++)
        {
            std::cout << innerVector[jdx];
            if (jdx < innerVector.size() - 1)
                std::cout << ", ";
        }

        std::cout << " ]";
        if (idx < bitPart.size() - 1)
            std::cout << ",";
        std::cout << "\n";
    }

#endif

    // Slots account for the positions in the deque in which to insert clocks.
    // 2 * unbounded.size() + 1 : the +1 accounts for the possibility of inserting clock sets before the first set (the one with index -l),
    //                            the 2* for the possibility to insert clock sets in between existing ones.
    const int totalSlots = static_cast<int>(2 * unboundedReg.size() + 1);

    // Now processing each partition.
    for (int i = 0; i < static_cast<int>(partitions.size()); i++)
    {
        const std::vector<boost::dynamic_bitset<>> &partition = bitPart[i];
        const int totalElementsInPartition = static_cast<int>(partition.size());

        // Now computing the total ways in which to insert elements in a deque.
        // TODO: vedere se si riesce a fare a meno di calcolare direttamente l'esponenziale.
        const auto total = static_cast<size_t>(std::pow(totalSlots, totalElementsInPartition));

#ifdef REGION_DEBUG

        std::cout << std::endl;
        std::cout << "REGION_DEBUG: totalElementsInPartition: " << totalElementsInPartition << "\n";
        std::cout << "REGION_DEBUG: Partition bitsets:\n";

        for (int jdx = 0; jdx < totalElementsInPartition; jdx++)
            std::cout << "              [" << jdx << "]: " << partition[jdx] << "\n";

#endif

        for (size_t j = 0; j < total; j++)
        {
            // Making this copy so that each newly generated Region can have its own unbounded sets.
            std::deque<boost::dynamic_bitset<>> newUnbounded = unboundedReg;

            // Variable used to identify the positions in which to insert partition elements.
            int ctr = static_cast<int>(j);

            // Keeps track of the clock sets which must be inserted last, otherwise the indices of the deque would be modified during the loop.
            absl::btree_map<int, std::vector<boost::dynamic_bitset<>>, std::greater<>> insertionOrder;

            for (int k = 0; k < totalElementsInPartition; k++)
            {
                // The index in which to insert the k-th element of a partition (need to be adjusted, see below).
                const int index = ctr % totalSlots;

                if (index % 2 == 0)
                    // Even index (also 0): the clock set must be inserted in-between existing ones, but we do it later to avoid modifying the indices during the loop.
                    insertionOrder[index / 2].emplace_back(partition[k]);
                else
                    // Odd index: we must insert clocks in an already existing set (we need to adjust idx since the deque does not contain x0).
                    newUnbounded.at((index - 1) / 2) |= partition[k];

#ifdef REGION_DEBUG

                std::cout << std::endl;
                std::cout << "REGION_DEBUG: Processing element " << k << " (bitset: " << partition[k] << ") at slot " << index << "\n";

#endif

                // Advancing the index.
                ctr /= totalSlots;
            }

#ifdef REGION_DEBUG

            std::cout << "REGION_DEBUG: Insertion order map:\n";
            for (auto &[key, vec]: insertionOrder)
            {
                std::cout << "              Key " << key << ":\n";
                for (size_t currentCombination = 0; currentCombination < vec.size(); currentCombination++)
                    std::cout << "                [" << currentCombination << "]: " << vec[currentCombination] << "\n";

                std::cout << "REGION_DEBUG: Now printing permutations:\n";

                std::ranges::sort(vec, [](const auto &a, const auto &b) { return a.to_ulong() < b.to_ulong(); });
                do
                {
                    std::cout << "              ";
                    for (const auto &bs: vec)
                        std::cout << bs << " ";
                    std::cout << "\n";
                } while (std::ranges::next_permutation(vec, [](const auto &a, const auto &b) { return a.to_ulong() < b.to_ulong(); }).found);
            }

#endif

            // Inserting missing clock sets in the deque.
            const std::vector<std::deque<boost::dynamic_bitset<>>> &permutedDeques = generateAllDeques(insertionOrder, newUnbounded);

#ifdef REGION_DEBUG

            std::cout << std::endl;
            std::cout << "REGION_DEBUG: permutedDeques (size " << permutedDeques.size() << "):\n";

            for (size_t idx = 0; idx < permutedDeques.size(); idx++)
            {
                std::cout << "              [" << idx << "]: ";
                for (const auto &bitset: permutedDeques[idx])
                    std::cout << bitset << " ";
                std::cout << "\n";
            }

#endif

            // For each obtained deque, we generate a new Region.
            for (const auto &permutedDeque: permutedDeques)
            {
                // Putting an empty map for variables placeholder, since in discrete predecessors integer variables are not considered.
                Region reg(qReg, H, permutedDeque, x0Reg, boundedReg, {});
                res.emplace_back(reg);
            }
        }
    }

    return res;
}


/**
 * @brief Auxiliary function computing the correct interval of values that clocks must take when computed as discrete predecessors.
 *
 * @param clockIndex the index of the current clock under analysis.
 * @param constraintOperator the constraint operator of the current clock constraint under analysis.
 * @param comparingConstant the integer constant of the current clock constraint under analysis.
 * @param maxConstant the maximum constant of the clock corresponding to clockIndex.
 * @param notInX0 a map where the key corresponds to a clock index and the value indicates that, when such a clock has such value,
 *                it must not be put in set x0 during discrete predecessors' computation.
 * @param notFractionalPart a map where the key corresponds to a clock index and the value indicates that, when such a clock has such value,
 *                         it must be put in set x0 during discrete predecessors' computation.
 * @param minMaxValues a map storing the minimum and maximum integer values a clock can take.
 */
inline void handleConstraintOperator(const int clockIndex,
                                     const comparison_op constraintOperator,
                                     const int comparingConstant,
                                     const int maxConstant,
                                     absl::flat_hash_map<int, int> &notInX0,
                                     absl::flat_hash_map<int, int> &notFractionalPart,
                                     absl::flat_hash_map<int, std::pair<int, int>> &minMaxValues)
{
    // Getting the minimum and maximum values a clock can take, if they already exist.
    std::pair<int, int> minMaxVal;
    if (minMaxValues.contains(clockIndex))
        minMaxVal = minMaxValues[clockIndex];

    // Here we directly put maxConstant + 1 when needed, as required by the algorithm in our paper.
    // See for parallelism the file ast.cpp, satisfaction of clock guards, to see how the fractional part of clocks is dealt with.
    switch (constraintOperator)
    {
        case GT:
        {
            const std::pair<int, int> interval = minMaxValues.contains(clockIndex)
                                                     ? std::make_pair(std::max(minMaxVal.first, comparingConstant),
                                                                      std::min(minMaxVal.second, maxConstant + 1))
                                                     : std::make_pair(comparingConstant, maxConstant + 1);
            minMaxValues[clockIndex] = interval;

            // We must not put the clock in x0 when the clock constraint is x > alpha and the clock is assigned value alpha (fractional part influences here).
            notInX0[clockIndex] = interval.first;
            break;
        }

        case GE:
        {
            const std::pair<int, int> interval = minMaxValues.contains(clockIndex)
                                                     ? std::make_pair(std::max(minMaxVal.first, comparingConstant),
                                                                      std::min(minMaxVal.second, maxConstant + 1))
                                                     : std::make_pair(comparingConstant, maxConstant + 1);
            minMaxValues[clockIndex] = interval;
            break;
        }

        case LT:
        {
            const std::pair<int, int> interval = minMaxValues.contains(clockIndex)
                                                     ? std::make_pair(std::max(minMaxVal.first, 0),
                                                                      std::min(minMaxVal.second, comparingConstant - 1))
                                                     : std::make_pair(0, comparingConstant - 1);
            minMaxValues[clockIndex] = interval;
            break;
        }

        case LE:
        {
            const std::pair<int, int> interval = minMaxValues.contains(clockIndex)
                                                     ? std::make_pair(std::max(minMaxVal.first, 0),
                                                                      std::min(minMaxVal.second, comparingConstant))
                                                     : std::make_pair(0, comparingConstant);
            minMaxValues[clockIndex] = interval;

            // We must put the clock in x0 when the clock constraint is x <= alpha and the clock is assigned value alpha (fractional part influences here).
            notFractionalPart[clockIndex] = interval.second;
            break;
        }

        default:
            std::cerr << "Not valid constraintOperator: " << constraintOperator << std::endl;
            break;
    }
}


std::vector<region::Region> region::Region::getImmediateDiscretePredecessors(const std::vector<transition> &transitions,
                                                                             const std::unordered_map<std::string, int> &clockIndices,
                                                                             const std::unordered_map<std::string, int> &locationsAsIntMap,
                                                                             const std::vector<int> &maxConstants) const
{
    std::vector<Region> res{};

    std::vector<std::pair<int, bool>> clockValuation = getClockValuation();

    for (const auto &transition: transitions)
    {
        // Handling the case in which no clocks are reset.
        if (transition.clocksToReset.empty())
        {
            // If no clocks must be reset, the clock valuation is the same, hence we can check whether the transition is satisfied before computing the new region.
            if (transition.isTransitionSatisfied(clockValuation, clockIndices, {}))
            {
                Region reg = clone();
                reg.set_q(locationsAsIntMap.at(transition.startingLocation));
                res.emplace_back(reg);
            }
        } else
        {
            const int numOfClocks = getNumberOfClocks();

            // Vector acting as the clock valuation (only integer values) for the clocks of the new regions.
            std::vector<int> H(numOfClocks);
            for (int i = 0; i < numOfClocks; i++)
                H[i] = h[i];

            boost::dynamic_bitset<> xBnd(numOfClocks);
            boost::dynamic_bitset<> xOob(numOfClocks);

            // Bitmask used to remove clocks after processing clock constraints.
            boost::dynamic_bitset<> xRefToDelete(numOfClocks);

            // Clocks not to be put in x0 in the new regions (the same as permRegs requires) when their value equals to the integer element of the map.
            // Key: clock index; value: the clock must not be put in x0 when assigned that value.
            absl::flat_hash_map<int, int> notInX0{};

            // Clocks to be put in x0 in the new region, e.g., when having a clock constraint like x <= alpha and trying to compute predecessors by assigning to x value alpha.
            // Key: clock index; value: the clock must be put in x0 when assigned that value.
            absl::flat_hash_map<int, int> notFractionalPart{};

            // The location of the new regions.
            const int qReg = locationsAsIntMap.at(transition.startingLocation);

            // Copying the unbounded, x0, and bounded attributes of the current region to pass them to permRegs.
            std::deque<boost::dynamic_bitset<>> newUnbounded = unbounded;
            boost::dynamic_bitset<> newX0 = x0;
            std::deque<boost::dynamic_bitset<>> newBounded = bounded;

#ifdef REGION_DEBUG

            std::cout << "\n\ngetImmediateDiscretePredecessor_DEBUG: qReg = " << qReg << std::endl;
            std::cout << "getImmediateDiscretePredecessor_DEBUG: numOfClocks = " << numOfClocks << std::endl;
            std::cout << "getImmediateDiscretePredecessor_DEBUG: H vector size = " << H.size() << std::endl;
            for (int i = 0; i < numOfClocks; i++)
                std::cout << "getImmediateDiscretePredecessor_DEBUG: H[" << i << "] = h[" << i << "] = " << H[i] << std::endl;

            std::cout << "getImmediateDiscretePredecessor_DEBUG: New unbounded: ";
            for (const auto &bitset: newUnbounded)
                std::cout << bitset << " ";
            std::cout << std::endl;

            std::cout << "getImmediateDiscretePredecessor_DEBUG: New x0: " << newX0 << std::endl;

            std::cout << "getImmediateDiscretePredecessor_DEBUG: New bounded: ";
            for (const auto &bitset: newBounded)
                std::cout << bitset << " ";
            std::cout << std::endl;

#endif

            // Getting the reset clocks as a bitset to ease the computation over clock constraints.
            boost::dynamic_bitset<> resetClocksBitset(numOfClocks);
            for (const std::string &clock: transition.clocksToReset)
                resetClocksBitset.set(cIdx(numOfClocks, clockIndices.at(clock)));

            // Bitmask for keeping track of the reset clocks after checking all clock constraints.
            boost::dynamic_bitset<> stillInReset(numOfClocks);

#ifdef REGION_DEBUG

            std::cout << "getImmediateDiscretePredecessor_DEBUG: resetClocksBitset: " << resetClocksBitset << std::endl;

#endif

            // Map collecting min and max values for the clocks that can be either bounded or unbounded.
            // Key: the index of the clock. Value: a pair <min_value_of_such_clock, max_value_for_such_clock>.
            absl::flat_hash_map<int, std::pair<int, int>> minMaxValues;

            // The paper now has been modified and directly checks all clocks that are reset.
            // Here, we still check all clock constraints first and then look for any clock that did not show up in a constraint but is still reset.
            // This implementation is CORRECT, as stated in the older version of the paper.
            for (const auto &[clock, constraintOperator, comparingConstant]: transition.clockGuard)
            {
                // Index of the clock in the current clock constraint.
                // ReSharper disable once CppTooWideScopeInitStatement
                const int clockIndex = clockIndices.at(clock);

                if (resetClocksBitset.test(cIdx(numOfClocks, clockIndex)))
                {
                    if (constraintOperator == EQ)
                        H[clockIndex] = comparingConstant;
                    else if (constraintOperator == GT && comparingConstant == maxConstants[clockIndex])
                    {
                        H[clockIndex] = maxConstants[clockIndex];
                        xOob.set(cIdx(numOfClocks, clockIndex));
                        xRefToDelete.set(cIdx(numOfClocks, clockIndex));
                    } else
                    {
                        xBnd.set(cIdx(numOfClocks, clockIndex));
                        xRefToDelete.set(cIdx(numOfClocks, clockIndex));

                        handleConstraintOperator(clockIndex,
                                                 constraintOperator,
                                                 comparingConstant,
                                                 maxConstants[clockIndex],
                                                 notInX0,
                                                 notFractionalPart,
                                                 minMaxValues);
                    }

                    // Removing the reset clock from the reset bitset to see if, at the end, any reset clock didn't show up in a guard.
                    stillInReset.set(cIdx(numOfClocks, clockIndex), true);
                }
            }

            resetClocksBitset &= ~stillInReset;

#ifdef REGION_DEBUG

            std::cout << "getImmediateDiscretePredecessor_DEBUG: resetClocksBitset after still in reset deletion: " << resetClocksBitset << std::endl;

#endif

            if (resetClocksBitset.any())
            {
                // Treating reset clocks that did not appear in a guard as having the implicit constraint clock_value >= 0.
                for (int i = 0; i < numOfClocks; i++)
                {
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const int pos = cIdx(numOfClocks, i);
                    if (resetClocksBitset.test(pos))
                    {
                        xBnd.set(pos);
                        xRefToDelete.set(pos);
                        minMaxValues[i] = std::make_pair(0, maxConstants[i] + 1);
                    }
                }
            }

            // Now removing clocks in xRefToDelete.
            for (auto &bitset: newUnbounded)
                bitset &= ~xRefToDelete;
            newX0 &= ~xRefToDelete;
            for (auto &bitset: newBounded)
                bitset &= ~xRefToDelete;

#ifdef REGION_DEBUG

            std::cout << "\n\ngetImmediateDiscretePredecessor_DEBUG AFTER DELETING xRefToDelete:" << std::endl;
            std::cout << "getImmediateDiscretePredecessor_DEBUG: qReg = " << qReg << std::endl;
            std::cout << "getImmediateDiscretePredecessor_DEBUG: numOfClocks = " << numOfClocks << std::endl;
            std::cout << "getImmediateDiscretePredecessor_DEBUG: H vector size = " << H.size() << std::endl;
            for (int i = 0; i < numOfClocks; i++)
                std::cout << "getImmediateDiscretePredecessor_DEBUG: H[" << i << "] = h[" << i << "] = " << H[i] << std::endl;

            std::cout << "getImmediateDiscretePredecessor_DEBUG: New unbounded: ";
            for (const auto &bitset: newUnbounded)
                std::cout << bitset << " ";
            std::cout << std::endl;

            std::cout << "getImmediateDiscretePredecessor_DEBUG: New x0: " << newX0 << std::endl;

            std::cout << "getImmediateDiscretePredecessor_DEBUG: New bounded: ";
            for (const auto &bitset: newBounded)
                std::cout << bitset << " ";
            std::cout << std::endl;

            std::cout << "getImmediateDiscretePredecessor_DEBUG: minMaxValues: { ";
            for (const auto &[clockIndex, minMaxPair]: minMaxValues)
                std::cout << clockIndex << ":(" << minMaxPair.first << "," << minMaxPair.second << ") ";
            std::cout << "}" << std::endl;

#endif

            if (xBnd.none())
            {
                // If all clocks do not require bounded partitioning, the clock valuation is the same as the current one, hence it suffices to compute the transition satisfiability once.
                // Temporary region used to check whether the new deques and x0 satisfy the transition guard.
                // ReSharper disable once CppTooWideScopeInitStatement
                const Region tmpReg(qReg, H, newUnbounded, newX0, newBounded, {});

                if (transition.isTransitionSatisfied(tmpReg.getClockValuation(), clockIndices, {}))
                {
                    std::vector<Region> discretePredecessors = permRegsUnbounded(qReg, H, newUnbounded, newX0, newBounded, numOfClocks, xOob);
                    res.insert(res.end(), discretePredecessors.begin(), discretePredecessors.end());
                }
            } else
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const std::vector<std::vector<std::pair<int, int>>> &intervalCombinations = generateAllIntegerIntervalCombinations(minMaxValues);

#ifdef REGION_DEBUG

                std::cout << "Now printing intervalCombinations" << std::endl;
                for (size_t i = 0; i < intervalCombinations.size(); ++i)
                {
                    std::cout << "Combination " << i << ": { ";

                    const auto &combination = intervalCombinations[i];
                    for (size_t j = 0; j < combination.size(); ++j)
                    {
                        const auto &[key, value] = combination[j];
                        std::cout << key << " -> " << value;
                        if (j + 1 < combination.size())
                            std::cout << ", ";
                    }

                    std::cout << " }" << std::endl;
                }

#endif

                for (const auto &combination: intervalCombinations)
                {
                    boost::dynamic_bitset<> delta(numOfClocks);
                    boost::dynamic_bitset<> notInX0Bitset(numOfClocks);
                    boost::dynamic_bitset<> notFractionalPartBitset(numOfClocks);
                    std::vector<int> HCopy = H;

                    for (const auto &[clockIdx, clockValue]: combination)
                    {
                        if (clockValue > maxConstants[clockIdx])
                            delta.set(cIdx(numOfClocks, clockIdx));

                        if (notInX0.contains(clockIdx) && notInX0.at(clockIdx) == clockValue)
                            notInX0Bitset.set(cIdx(numOfClocks, clockIdx));

                        if (notFractionalPart.contains(clockIdx) && notFractionalPart.at(clockIdx) == clockValue)
                            notFractionalPartBitset.set(cIdx(numOfClocks, clockIdx));

                        HCopy[clockIdx] = clockValue > maxConstants[clockIdx] ? maxConstants[clockIdx] : clockValue;
                    }

                    boost::dynamic_bitset<> deltaUnionOob = delta | xOob;
                    boost::dynamic_bitset<> xBndMinusDelta = xBnd & ~delta;

                    std::vector<Region> tmp{};

                    tmp = permRegsUnbounded(qReg, HCopy, newUnbounded, newX0, newBounded, numOfClocks, deltaUnionOob);

                    if (!tmp.empty() && xBndMinusDelta.any())
                    {
                        for (const auto &reg: tmp)
                        {
                            std::vector<Region> tmp2 = permRegsBounded(reg.q,
                                                                       HCopy,
                                                                       reg.unbounded,
                                                                       reg.x0,
                                                                       reg.bounded,
                                                                       numOfClocks,
                                                                       xBndMinusDelta,
                                                                       maxConstants,
                                                                       notInX0Bitset,
                                                                       notFractionalPartBitset);

                            // TODO: vedere se con OpenMP migliora.
                            for (const auto &reg2: tmp2)
                                if (transition.isTransitionSatisfied(reg2.getClockValuation(), clockIndices, {}))
                                    res.emplace_back(reg2);
                        }
                    } else
                    {
                        // ReSharper disable once CppTooWideScopeInitStatement
                        const Region tmpReg(qReg, HCopy, newUnbounded, newX0, newBounded, {});
                        if (transition.isTransitionSatisfied(tmpReg.getClockValuation(), clockIndices, {}))
                            res.insert(res.end(), tmp.begin(), tmp.end());
                    }
                }
            }
        }
    }

    return res;
}


std::string region::Region::toString() const
{
    std::ostringstream oss;
    oss << "Region\n";
    oss << "  q: " << q << "\n";
    oss << "  h: [";
    if (!h.empty())
        for (const int el: h)
            oss << "  " << el;
    oss << "  ]\n";
    oss << "  unbounded: [";
    for (const auto &i: unbounded)
        oss << "  " << i;
    oss << "  ]\n";
    oss << "  x0:        [  " << x0 << "  ]\n";
    oss << "  bounded:   [";
    for (const auto &i: bounded)
        oss << "  " << i;
    oss << "  ]\n";
    bool first2 = true;
    oss << "  variables: [";
    for (const auto &[fst, snd]: variables)
    {
        if (!first2)
            oss << ", ";
        first2 = false;
        oss << fst << " -> " << snd;
    }
    oss << "]\n";
    return oss.str();
}


size_t region::Region::printSizeInBytes(const bool printStats) const
{
    // Start with the base size of the Region object itself.
    size_t totalSize = sizeof(*this);

    // Size of the h vector (clock integer values) - dynamically allocated.
    // Use capacity() instead of size() to get actual allocated memory.
    const size_t sizeOfH = h.capacity() * sizeof(int);
    totalSize += sizeOfH;

    // Size of unbounded deque (bitsets) - dynamic storage.
    // Add the actual bits' storage (overhead already counted in sizeof(*this)).
    size_t sizeOfUnbounded = 0;
    for (const auto &bitset: unbounded)
        sizeOfUnbounded += bitset.num_blocks() * sizeof(boost::dynamic_bitset<>::block_type);
    totalSize += sizeOfUnbounded;

    // Size of x0 bitset - dynamic storage.
    const size_t sizeOfX0 = x0.num_blocks() * sizeof(boost::dynamic_bitset<>::block_type);
    totalSize += sizeOfX0;

    // Size of bounded deque (bitsets) - dynamic storage.
    size_t sizeOfBounded = 0;
    // Add the actual bits' storage (overhead already counted in sizeof(*this)).
    for (const auto &bitset: bounded)
        sizeOfBounded += bitset.num_blocks() * sizeof(boost::dynamic_bitset<>::block_type);
    totalSize += sizeOfBounded;

    // Size of variables map - dynamic storage.
    // String capacity includes allocated memory.
    // Note: sizeof(varValue) is already included in sizeof(*this) for the map structure.
    size_t sizeOfVariables = 0;
    for (const auto &varName: variables | std::views::keys)
        sizeOfVariables += varName.capacity();
    totalSize += sizeOfVariables;

    if (printStats)
    {
        std::cout << "Region Size Breakdown (in bytes):\n";
        std::cout << "  Base object size:    " << sizeof(*this) << "\n";
        std::cout << "  H vector:            " << sizeOfH << "\n";
        std::cout << "  Unbounded deque:     " << sizeOfUnbounded << "\n";
        std::cout << "  X0 bitset:           " << sizeOfX0 << "\n";
        std::cout << "  Bounded deque:       " << sizeOfBounded << "\n";
        std::cout << "  Variables map:       " << sizeOfVariables << "\n";
        std::cout << "  ---------------------------------\n";
        std::cout << "  Total size:          " << totalSize << " bytes\n";
    }

    return totalSize;
}
