#ifndef TARZAN_HASH_UTILITIES_H
#define TARZAN_HASH_UTILITIES_H

#include <cstddef>
#include <string>
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>


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
 * @brief Hashes a boost::dynamic_bitset using Boost's optimized hash function.
 *
 * @param bitset the boost::dynamic_bitset to hash.
 * @return the hash of bitset.
 */
inline std::size_t hash_bitset(const boost::dynamic_bitset<> &bitset)
{
    return boost::hash<boost::dynamic_bitset<>>()(bitset);
}

#endif //TARZAN_HASH_UTILITIES_H
