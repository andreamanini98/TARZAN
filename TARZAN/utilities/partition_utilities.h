#ifndef TARZAN_PARTITION_UTILITIES_H
#define TARZAN_PARTITION_UTILITIES_H

#include <vector>
#include <numeric>
#include <boost/dynamic_bitset.hpp>
#include "absl/container/flat_hash_map.h"
#include "absl/container/btree_map.h"

#include "TARZAN/utilities/function_utilities.h"

// #define PARTITION_DEBUG

using insOrdMap = absl::btree_map<int, std::vector<boost::dynamic_bitset<>>, std::greater<>>;
using dequeVector = std::vector<std::deque<boost::dynamic_bitset<>>>;
using permutationsCache = absl::flat_hash_map<int, std::vector<std::vector<boost::dynamic_bitset<>>>>;


/**
 * @brief Used to retrieve a singleton acting as a cache for permutations.
 *
 * @return an absl::flat_hash_map acting as a cache for stored partitions.
 */
inline permutationsCache &get_p_cache()
{
    static permutationsCache cache;
    return cache;
}


/**
 * @brief The implementation follows the paper: Maximize the Rightmost Digit: Gray Codes for Restricted Growth Strings.
 *
 * A partition is a Restricted Growth String (or Bell String), denoting the set in which elements must belong.
 * Elements are represented by their integer index, since we only want to partition elements in the bitset.
 * For example, if we have bitset = 10010, then the elements to partition correspond to indices 0 and 3.
 *
 * @param bitset the bitset to partition (only partitions the bits set to 1).
 * @param partitions a std::vector in which to store the partitions of bitset.
 * @param activeBitsIndices a std::vector in which to store the indices of the elements that have been partitioned.
 */
inline void partitionBitset(const boost::dynamic_bitset<> &bitset, std::vector<std::vector<int>> &partitions, std::vector<int> &activeBitsIndices)
{
    // Collecting the indices of the elements to partition.
    const int bitsetSize = static_cast<int>(bitset.size());

    for (int i = 0; i < bitsetSize; i++)
        if (bitset.test(cIdx(bitsetSize, i)))
            activeBitsIndices.push_back(i);

#ifdef PARTITION_DEBUG

    std::cout << "The bitset indices are:" << std::endl;
    for (const int activeBitsIndice: activeBitsIndices)
        std::cout << activeBitsIndice << " ";
    std::cout << std::endl;

#endif

    // Initializing auxiliary data structures.
    const int totalActiveBits = static_cast<int>(activeBitsIndices.size());
    std::vector vecA(totalActiveBits, 0);
    std::vector<int> vecF(totalActiveBits + 1);
    std::iota(vecF.begin(), vecF.end(), 0);
    std::vector vecS(totalActiveBits, 0);
    std::vector<int> stackS{}; // Vector used as a stack.
    std::vector vecV(totalActiveBits, true);

#ifdef PARTITION_DEBUG

    // Print all vectors.
    std::cout << "Initial values before computation:\n";
    std::cout << "totalActiveBits = " << totalActiveBits << "\n";

    // Print vecA.
    std::cout << "vecA (size " << vecA.size() << "): [";
    for (size_t i = 0; i < vecA.size(); ++i)
    {
        std::cout << vecA[i];
        if (i < vecA.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";

    // Print vecF.
    std::cout << "vecF (size " << vecF.size() << "): [";
    for (size_t i = 0; i < vecF.size(); ++i)
    {
        std::cout << vecF[i];
        if (i < vecF.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";

    // Print vecS.
    std::cout << "vecS (size " << vecS.size() << "): [";
    for (size_t i = 0; i < vecS.size(); ++i)
    {
        std::cout << vecS[i];
        if (i < vecS.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";

    // Print stackS.
    std::cout << "stackS (size " << stackS.size() << "): [";
    if (stackS.empty())
    {
        std::cout << "empty";
    } else
    {
        // Print from bottom to top (the first element = bottom).
        for (size_t i = 0; i < stackS.size(); ++i)
        {
            std::cout << stackS[i];
            if (i < stackS.size() - 1)
                std::cout << ", ";
        }
    }
    std::cout << "]\n";

    // Print vecV.
    std::cout << "vecV (size " << vecV.size() << "): [";
    for (size_t i = 0; i < vecV.size(); ++i)
    {
        std::cout << (vecV[i] ? "true" : "false");
        if (i < vecV.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";

#endif

    // Saving the trivial partition.
    partitions.emplace_back(vecA);

    int m{};
    int idx{};

    // Now starting the computation of partitions.
    const int iterationThreshold = totalActiveBits - 1;

    while (vecF[0] < iterationThreshold)
    {
        idx = vecF[0];
        vecF[0] = 0;

        if (vecA[idx] == vecS[idx])
        {
            if (vecV[idx])
            {
                m = 0;
                vecV[idx] = false;
            } else if (stackS.empty())
                m = 1;
            else
                m = vecA[stackS.back()];

            vecA[idx] = m + 1;
            if (m + 1 != 1)
                stackS.push_back(idx);
        } else if (vecA[idx] == 2 && vecS[idx] == 1)
        {
            vecA[idx] -= 2;
            if (stackS.back() == idx)
                stackS.pop_back();
        } else
        {
            vecA[idx] -= 1;
            if (stackS.back() == idx)
                stackS.pop_back();
        }

        partitions.emplace_back(vecA);

        if (vecA[idx] == 1 - vecS[idx])
        {
            vecF[idx] = vecF[idx + 1];
            vecF[idx + 1] = idx + 1;
            vecS[idx] = vecA[idx];
        }
    }

#ifdef PARTITION_DEBUG

    std::cout << "\nCollected partitions" << " (size " << partitions.size() << "): ";
    if (partitions.empty())
    {
        std::cout << "[empty]\n";
    } else
    {
        std::cout << "[\n";
        for (size_t i = 0; i < partitions.size(); ++i)
        {
            std::cout << "  Partition " << (i + 1) << ": [";
            for (size_t j = 0; j < partitions[i].size(); ++j)
            {
                std::cout << partitions[i][j];
                if (j < partitions[i].size() - 1) std::cout << ", ";
            }
            std::cout << "]";
            if (i < partitions.size() - 1) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << "]\n";
    }

#endif
}


/**
 * @brief Computes the bitsets derived from Restricted Growth String partitions.
 *
 * @param bitsetSize the size of the resulting bitset (this is done to account for the elements contained in activeBitsIndices).
 * @param partitions the partitions represented as Restricted Growth Strings.
 * @param activeBitsIndices the indices of the elements belonging to the partitions.
 * @return a std::vector of partitions represented as dynamic bitsets.
 */
inline std::vector<std::vector<boost::dynamic_bitset<>>> getBitsetsFromRestrictedGrowthStrings(const int bitsetSize,
                                                                                               const std::vector<std::vector<int>> &partitions,
                                                                                               const std::vector<int> &activeBitsIndices)
{
    const int totalPartitions = static_cast<int>(partitions.size());
    const int totalElementsInPartition = static_cast<int>(activeBitsIndices.size());

    std::vector<std::vector<boost::dynamic_bitset<>>> res;
    res.resize(totalPartitions);

    // This loop can be parallelized with OpenMP. However, speedup appears only when there are a LARGE number of partitions (otherwise, you get a slowdown).
    for (int i = 0; i < totalPartitions; i++)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const std::vector<int> &partition = partitions[i];

        if (!partition.empty())
        {
            auto it = std::ranges::max_element(partition);
            const int maxNumberOfSets = *it;

            std::vector<boost::dynamic_bitset<>> bitsetPartition{};
            bitsetPartition.reserve(maxNumberOfSets);

            for (int j = 0; j < maxNumberOfSets + 1; j++)
                bitsetPartition.emplace_back(bitsetSize);

            for (int j = 0; j < totalElementsInPartition; j++)
                bitsetPartition[partition[j]].set(cIdx(bitsetSize, activeBitsIndices[j]));

            res[i] = std::move(bitsetPartition);
        }
    }

    return res;
}


/**
 * @brief Computes and collects all possible permutations of all std::vector elements.
 *
 * @tparam T the type of the vector.
 * @param vec the vector that must be permuted.
 * @return all possible permutations of vec elements.
 */
template<typename T>
std::vector<std::vector<T>> getAllVectorPermutations(const std::vector<T> &vec)
{
    std::vector<std::vector<T>> res;

    std::vector<T> tmp = vec;
    std::sort(tmp.begin(), tmp.end());

    do
        res.emplace_back(tmp);
    while (std::next_permutation(tmp.begin(), tmp.end()));

    return res;
}


/**
 * @brief Recursively computes all possible deques from inserting all permutations of the map elements in positions described by map keys.
 *
 * @param map the integer key denotes the position of the deque 'inputDeque' in which to insert all possible permutations of the key values (vectors).
 *            Note that keys are stored from biggest to smallest; in this way, inserting from the biggest one does not influence indexing.
 * @param current the current deque from which computation must start.
 * @param it an iterator moving through the map.
 * @param output a std::vector containing all deques.
 */
inline void generateDeques(const insOrdMap &map, const std::deque<boost::dynamic_bitset<>> &current, const insOrdMap::const_iterator it, dequeVector &output)
{
    if (it == map.end())
    {
        output.emplace_back(current);
        return;
    }

    auto &p_cache = get_p_cache();

    const int pos = it->first;
    const auto &vec = it->second;

    // Trying to cache the permutation if not already in cache.
    auto [it_cache, inserted] = p_cache.try_emplace(pos, getAllVectorPermutations(vec));
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto &permutations = it_cache->second;

#ifdef PARTITION_DEBUG

    if (inserted)
        std::cout << "Ins: Inserted permutation in cache with key: " << pos << std::endl;
    else
        std::cout << "NoIns: Not inserted permutation in cache with key: " << pos << std::endl;

#endif

    for (const auto &perm: permutations)
    {
        // Insert permutation into current deque at position 'pos'.
        std::deque<boost::dynamic_bitset<>> next = current;
        next.insert(next.begin() + pos, perm.begin(), perm.end());

        generateDeques(map, next, std::next(it), output);
    }
}


/**
 * @brief Computes all possible deques from inserting all permutations of the map elements in positions described by map keys.
 *
 * @param map the integer key denotes the position of the deque 'inputDeque' in which to insert all possible permutations of the key values (vectors).
 *            Note that keys are stored from biggest to smallest; in this way, inserting from the biggest one does not influence indexing.
 * @param inputDeque the deque in which to insert permutations.
 * @return a std::vector of deques obtained by inserting, at specific positions, all permutations of vectors (as described by the map parameter).
 */
// TODO: se parallelizzi il loop in Region.cpp, hai delle race conditions quando accedi alla cache dato che è statica.
//       Non so se per risolvere basta rendere la cache una variabile locale a generateAllDeques per poi passarla come parametro, ma così non funzionava correttamente
//       il salvataggio degli elementi all'interno della cache.
inline dequeVector generateAllDeques(const insOrdMap &map, const std::deque<boost::dynamic_bitset<>> &inputDeque)
{
    dequeVector res;

    generateDeques(map, inputDeque, map.begin(), res);

    // We need to free the cache after all deques have been generated so that it can be reused when invoked again.
    get_p_cache().clear();

    return res;
}


/**
 * @brief Computes all the possible combinations of the integer intervals values as given by the minMaxValues map.
 *
 * @param minMaxValues map storing, for each key, the integer interval the element corresponding to such a key can take (an integer in such an interval).
 * @return the cartesian product of all possible values belonging to the intervals.
 */
inline std::vector<std::vector<std::pair<int, int>>> generateAllIntegerIntervalCombinations(const absl::flat_hash_map<int, std::pair<int, int>> &minMaxValues)
{
    std::vector<std::vector<std::pair<int, int>>> result{};

    if (minMaxValues.empty())
        return result;

    // Extract keys and ranges for easier iteration.
    std::vector<int> keys;
    std::vector<std::pair<int, int>> ranges;

    for (const auto &[key, range]: minMaxValues)
    {
        keys.push_back(key);
        ranges.push_back(range);
    }

    // Calculate the total number of combinations.
    size_t totalCombinations = 1;
    for (const auto &[min_val, max_val]: ranges)
        totalCombinations *= max_val - min_val + 1;

    result.reserve(totalCombinations);

    // Generate all combinations using a counter-approach.
    for (size_t i = 0; i < totalCombinations; i++)
    {
        std::vector<std::pair<int, int>> combination;
        combination.reserve(keys.size());

        size_t temp = i;
        for (size_t j = 0; j < keys.size(); j++)
        {
            const int min_val = ranges[j].first;
            const int max_val = ranges[j].second;
            const int range_size = max_val - min_val + 1;

            int value = static_cast<int>(min_val + temp % range_size);
            combination.emplace_back(keys[j], value);

            temp /= range_size;
        }

        result.push_back(std::move(combination));
    }

    return result;
}


/**
 * @brief Computes the cartesian product of the given vectors.
 *
 * @param input a std::vector of std::vectors, each of which contains integers.
 * @return the cartesian product of every vector's integer values inside the input vector.
 */
inline std::vector<std::vector<int>> vectorsCartesianProduct(const std::vector<std::vector<int>> &input)
{
    std::vector<std::vector<int>> result;

    if (input.empty())
        return result;

    // Start with an empty combination.
    result.emplace_back();

    for (const auto &inner: input)
    {
        std::vector<std::vector<int>> temp;
        for (const auto &combination: result)
        {
            for (const int value: inner)
            {
                auto newComb = combination;
                newComb.push_back(value);
                temp.push_back(std::move(newComb));
            }
        }
        result = std::move(temp);
    }

    return result;
}

#endif //TARZAN_PARTITION_UTILITIES_H
