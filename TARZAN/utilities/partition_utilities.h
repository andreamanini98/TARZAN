#ifndef TARZAN_PARTITION_UTILITIES_H
#define TARZAN_PARTITION_UTILITIES_H

#include <vector>
#include <boost/dynamic_bitset.hpp>

#include "TARZAN/utilities/function_utilities.h"

// #define PARTITION_DEBUG


/**
 * @brief The implementation follows the paper: Maximize the Rightmost Digit: Gray Codes for Restricted Growth Strings.
 *
 * A partition is a Restricted Growth String (or Bell String), denoting the set in which elements must belong.
 * Elements are represented by their integer index, since we only want to partition elements in the bitset.
 * For example, if we have bitset = 10010, then the elements to partition correspond to indices 0 and 3.
 *
 * @param bitset the bitset to partition (only partitions the bits set to 1).
 * @return all partitions of the bitset parameter.
 *         In particular, the first element of the returned pair contains the partitions, while the second element contains the vector specifying
 *         the indices of the elements that have been partitioned.
 */
inline std::pair<std::vector<std::vector<int>>, std::vector<int>> partitionBitset(const boost::dynamic_bitset<> &bitset)
{
    // Collecting the indices of the elements to partition.
    std::vector<int> activeBitsIndices{};
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


    std::vector<std::vector<int>> partitions;

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


    return { partitions, activeBitsIndices };
}

#endif //TARZAN_PARTITION_UTILITIES_H
