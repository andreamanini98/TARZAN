#ifndef TARZAN_HASH_UTILITIES_H
#define TARZAN_HASH_UTILITIES_H

#include <cstddef>
#include <string>
#include <boost/dynamic_bitset.hpp>


/**
 * @brief Combines two hashes using golden ratio hashing.
 *
 * @tparam T the class of the new element for which we are combining the hash.
 * @param seed an existing hash value.
 * @param v an element whose hash value must be combined with seed.
 */
template<class T>
void hash_combine(std::size_t &seed, const T &v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


/**
 * @brief Hashes a boost::dynamic_bitset exploiting its string representation.
 *
 * @param bitset the boost::dynamic_bitset to hash.
 * @return the hash of bitset.
 */
inline std::size_t hash_bitset(const boost::dynamic_bitset<> &bitset)
{
    std::size_t seed = 0;
    hash_combine(seed, bitset.size());

    std::string bit_string;
    boost::to_string(bitset, bit_string);
    hash_combine(seed, std::hash<std::string>{}(bit_string));

    return seed;
}

#endif //TARZAN_HASH_UTILITIES_H
