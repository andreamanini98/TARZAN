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
                    resetClocksMask.set(numOfClocks - 1 - resetClockIdx); // TODO: usare l'inline function anche qui
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


// TODO: sembrerebbe che molte regioni vengono calcolate (correttamente) più volte durante tutte le possibili combinazioni, magari si può semplificare.
// TODO: DEVI IMPLEMENTARE IL CASO UNBOUNDED E OKKIO SE HAI UNA DEQUE VUOTA (PER IL CASO BOUNDED NON SI VERIFICA QUESTO PROBLEMA PERCHÈ CONSIDERI SEMPRE X0).
std::vector<region::Region> region::Region::permRegs(const bool partBounded, boost::dynamic_bitset<> X, const int maxConstant, const std::vector<int> &H) const
{
    std::vector<Region> res{};
    const int numOfClocks = getNumberOfClocks();

    // Bitset collecting clocks that must directly go in X0 in the new regions (if considering only bounded clocks).
    boost::dynamic_bitset<> newX0(numOfClocks);

    // Checking if clocks can be inserted into X0 to ease the partitioning of set X.
    if (partBounded)
    {
        boost::dynamic_bitset<> cMaxClockMask(numOfClocks);

        for (int i = 0; i < numOfClocks; i++)
            if (X.test(cIdx(numOfClocks, i)) && H[i] == maxConstant)
                cMaxClockMask.set(cIdx(numOfClocks, i));

        newX0 |= cMaxClockMask;
        cMaxClockMask.flip();
        X &= cMaxClockMask;
    }

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

    // Slots account for the positions in the deques in which to insert clocks.
    // 2 * (bounded.size() + 1) : the +1 accounts for X0, the 2* for the possibility to insert clock sets in between existing ones.
    // 2 * unbounded.size() + 1 : the +1 accounts for the possibility of inserting clock sets before the first set (the one with index -l), the 2* as above.
    const int totalSlots = partBounded ? static_cast<int>(2 * (bounded.size() + 1)) : static_cast<int>(2 * unbounded.size() + 1);

    // Now processing each partition.
    for (int i = 0; i < static_cast<int>(partitions.size()); i++)
    {
        const std::vector<boost::dynamic_bitset<>> &partition = bitPart[i];
        const int totalElementsInPartition = static_cast<int>(partition.size());

        // Now computing the total ways in which to insert elements in a dequeue.
        const auto total = static_cast<size_t>(std::pow(totalSlots, totalElementsInPartition));

#ifdef REGION_DEBUG

        std::cout << std::endl;
        std::cout << "REGION_DEBUG: totalElementsInPartition: " << totalElementsInPartition << "\n";
        std::cout << "REGION_DEBUG: Partition bitsets:\n";

        for (int jdx = 0; jdx < totalElementsInPartition; jdx++)
            std::cout << "              [" << jdx << "]: " << partition[jdx] << "\n";

#endif

        if (partBounded)
        {
            for (size_t j = 0; j < total; j++)
            {
                Region reg = clone();
                reg.x0 |= newX0;

                // Variable used to identify the positions in which to insert partition elements.
                int ctr = static_cast<int>(j);

                // Keeps track of the clock sets which must be inserted last, otherwise the indices of the deque would be modified during the loop.
                absl::btree_map<int, std::vector<boost::dynamic_bitset<>>, std::greater<>> insertionOrder;

                for (int k = 0; k < totalElementsInPartition; k++)
                {
                    // The index in which to insert the k-th element of a partition (need to be adjusted, see below).
                    const int index = ctr % totalSlots;

                    if (index == 0)
                        // Index 0: we must insert the clocks in X0.
                        reg.x0 |= partition[k];
                    else if (index % 2 == 0)
                        // Even index: we must insert clocks in an already existing set (we need to adjust idx since the deque does not contain X0).
                        reg.bounded.at((index - 1) / 2) |= partition[k];
                    else
                        // Odd index: the clock set must be inserted in-between existing ones, but we do it later to avoid modifying the indices during the loop.
                        insertionOrder[index / 2].emplace_back(partition[k]);

#ifdef REGION_DEBUG

                    std::cout << std::endl;
                    std::cout << "REGION_DEBUG: Processing element " << k << " (bitset: " << partition[k] << ") at slot " << index << "\n";

#endif

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
                const std::vector<std::deque<boost::dynamic_bitset<>>> &permutedDeques = generateAllDeques(insertionOrder, reg.bounded);

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

                for (const auto &permutedDeque: permutedDeques)
                {
                    reg.set_bounded(permutedDeque);

                    for (int k = 0; k < static_cast<int>(H.size()); k++)
                        reg.h[k] = H[k];

                    res.emplace_back(reg);
                }
            }
        }
    }

    return res;
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
