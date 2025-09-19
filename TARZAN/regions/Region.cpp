#include "Region.h"

#include <sstream>
#include <iostream>
#include "absl/container/btree_map.h"

#include "TARZAN/utilities/function_utilities.h"
#include "TARZAN/utilities/partition_utilities.h"

#define REGION_DEBUG


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


region::Region region::Region::getImmediateDelaySuccessor(const int maxConstant) const
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
                if (reg.h[i] == maxConstant)
                    xOob.set(cIdx(numOfClocks, i), true);
                else
                    xTmp.set(cIdx(numOfClocks, i), true);
            }
        }
        reg.x0 &= boost::dynamic_bitset<>(numOfClocks);

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

    const std::vector<std::pair<int, bool>> clockValuation = getClockValuation();

    for (const auto &transition: transitions)
    {
        if (transition.isGuardSatisfied(clockValuation, clockIndices))
        {
            Region reg = clone();
            reg.set_q(locationsAsIntMap.at(transition.targetLocation));

            if (!transition.clocksToReset.empty())
            {
                const int numOfClocks = getNumberOfClocks();
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

            res.push_back(reg);
        }
    }

    return res;
}


/**
 * @brief Auxiliary function computing the permRegs function as seen in our paper for the bounded case.
 *
 * @param q the location of the Region for which we are computing discrete predecessors.
 * @param H a vector containing the values of the clocks (indices are computed as usual for Timed Automata, see the ast.h file).
 * @param unbounded the unbounded sets of the Region for which we are computing discrete predecessors.
 * @param x0 the x0 set of the Region for which we are computing discrete predecessors.
 * @param bounded the bounded sets of the Region for which we are computing discrete predecessors.
 * @param numOfClocks the original Timed Automaton number of clocks.
 * @param X the clocks for which we must compute all ordered partitions.
 * @param maxConstant the maximum constant of the original Timed Automaton.
 * @return all regions returned by permRegs as described in our paper.
 */
inline std::vector<region::Region> permRegsBounded(const int q,
                                                   const std::vector<int> &H,
                                                   const std::deque<boost::dynamic_bitset<>> &unbounded,
                                                   boost::dynamic_bitset<> x0,
                                                   const std::deque<boost::dynamic_bitset<>> &bounded,
                                                   const int numOfClocks,
                                                   boost::dynamic_bitset<> X,
                                                   const int maxConstant)
{
    std::vector<region::Region> res{};

    // Checking if clocks of X can be inserted into x0 to ease the partitioning of set X.
    boost::dynamic_bitset<> maxConstantClockMask(numOfClocks);

    for (int i = 0; i < numOfClocks; i++)
        if (X.test(cIdx(numOfClocks, i)) && H[i] == maxConstant)
            maxConstantClockMask.set(cIdx(numOfClocks, i));

    x0 |= maxConstantClockMask;
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
    const int totalSlots = static_cast<int>(2 * (bounded.size() + 1));

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
            boost::dynamic_bitset<> newX0 = x0;
            std::deque<boost::dynamic_bitset<>> newBounded = bounded;

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
                region::Region reg(q, H, unbounded, newX0, permutedDeque);
                res.emplace_back(reg);
            }
        }
    }

    return res;
}


/**
 * @brief Auxiliary function computing the permRegs function as seen in our paper for the unbounded case.
 *
 * @param q the location of the Region for which we are computing discrete predecessors.
 * @param H a vector containing the values of the clocks (indices are computed as usual for Timed Automata, see the ast.h file).
 * @param unbounded the unbounded sets of the Region for which we are computing discrete predecessors.
 * @param x0 the x0 set of the Region for which we are computing discrete predecessors.
 * @param bounded the bounded sets of the Region for which we are computing discrete predecessors.
 * @param numOfClocks the original Timed Automaton number of clocks.
 * @param X the clocks for which we must compute all ordered partitions.
 * @return all regions returned by permRegs as described in our paper.
 */
inline std::vector<region::Region> permRegsUnbounded(const int q,
                                                     const std::vector<int> &H,
                                                     const std::deque<boost::dynamic_bitset<>> &unbounded,
                                                     const boost::dynamic_bitset<> &x0,
                                                     const std::deque<boost::dynamic_bitset<>> &bounded,
                                                     const int numOfClocks,
                                                     const boost::dynamic_bitset<> &X)
{
    std::vector<region::Region> res{};

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
    const int totalSlots = static_cast<int>(2 * unbounded.size() + 1);

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
            std::deque<boost::dynamic_bitset<>> newUnbounded = unbounded;

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
                region::Region reg(q, H, permutedDeque, x0, bounded);
                res.emplace_back(reg);
            }
        }
    }

    return res;
}


// TODO: sembrerebbe che molte regioni vengono calcolate (correttamente) più volte durante tutte le possibili combinazioni, magari si può semplificare.
std::vector<region::Region> region::Region::permRegs(const bool partBounded, const boost::dynamic_bitset<> &X, const int cMax, const std::vector<int> &H) const
{
    if (partBounded)
        return permRegsBounded(q, H, unbounded, x0, bounded, getNumberOfClocks(), X, cMax);

    return permRegsUnbounded(q, H, unbounded, x0, bounded, getNumberOfClocks(), X);
}


std::vector<region::Region> region::Region::getImmediateDiscretePredecessors(const std::vector<transition> &transitions,
                                                                             const std::unordered_map<std::string, int> &clockIndices,
                                                                             const std::unordered_map<std::string, int> &locationsAsIntMap) const
{
    std::vector<Region> res{};

    return res;
}


std::string region::Region::toString() const
{
    std::ostringstream oss;
    oss << "Region\n";
    oss << "  q: " << q << "\n";
    oss << "  h: [";
    if (h != nullptr)
    {
        const int *it = h;
        for (int i = 0; i < static_cast<int>(x0.size()); i++)
        {
            oss << "  " << *it;
            it++;
        }
    }
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
    return oss.str();
}
