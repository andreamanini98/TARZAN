#ifndef TARZAN_NETWORKREGION_H
#define TARZAN_NETWORKREGION_H

#include <vector>
#include "absl/container/btree_set.h"
#include "absl/container/btree_map.h"

#include "TARZAN/regions/Region.h"


// An advantage of defining the configuration of the network using a vector of regions is that, when computing discrete successors, you can compute
// them only on the necessary regions, instead of computing the discrete successor of the whole cartesian product region.

namespace networkOfTA
{
    class NetworkRegion
    {
        /// Each region corresponds to a Timed Automaton of the network.
        std::vector<region::Region> regions{};

        /// Contains the indices of the regions vector such that the corresponding region is either of class A or class C.
        absl::btree_set<int> isAorC{};

        /**
         * Keeps track of the clocks ordering between different regions. Key = region index.
         * The maps in the front capture the smallest equivalent fractional parts, while the maps in the back capture the highest equivalent fractional parts.
         */
        // TODO: vedere se una deque è la struttura dati più adeguata considerando che poi devi cancellare le mappe che sono vuote.
        std::deque<absl::btree_map<int, boost::dynamic_bitset<>>> clockOrdering{};


    public:
        explicit NetworkRegion(const std::vector<region::Region> &regions, const bool allRegionsAreInitial) : regions(regions)
        {
            // If every region is initial, they must all belong to class A.
            if (allRegionsAreInitial)
            {
                const int numRegions = static_cast<int>(regions.size());

                // We insert values from 0 to numRegions - 1 since every region is initial.
                for (int i = 0; i < numRegions; i++)
                    isAorC.insert(i);
            }
        }


        // Copy constructor.
        NetworkRegion(const NetworkRegion &other) = default;


        /**
         * @brief Computes the immediate delay successor of the current network region.
         *
         * @param maxConstants the maximum constants of the Timed Automata from which the network region is derived.
         * @return a NetworkRegion immediate delay successor of the current network region.
         */
        [[nodiscard]] NetworkRegion getImmediateDelaySuccessor(const std::vector<int> &maxConstants) const;


        /**
         * @brief Computes the immediate discrete successor of the current network region.
         *
         * @param transitions the (vector of) transitions over which immediate discrete successors must be computed.
         * @param clockIndices the (vector of) indices of the clocks as they appear in the clocks vector of a Timed Automaton.
         * @param locationsToInt a (vector of) std::unordered_map associating an integer with each string name.
         * @return a std::vector<NetworkRegion> containing immediate discrete successors of the current network region.
         *         If no successors can be computed, returns an empty std::vector.
         *
         * @warning The transitions parameter must contain all and only the transitions exiting from the location of every region.
         *          For this reason, it must contain an empty vector at position i if the i-th automaton has no outgoing transitions
         *          from its current location, otherwise index ordering will be lost.
         */
        [[nodiscard]] std::vector<NetworkRegion> getImmediateDiscreteSuccessors(
            const std::vector<std::vector<transition>> &transitions,
            const std::vector<std::unordered_map<std::string, int>> &clockIndices,
            const std::vector<std::unordered_map<std::string, int>> &locationsToInt) const;


        /**
         * @brief Creates a deep copy of this region.
         *
         * @return a new NetworkRegion object that is a copy of this one.
         */
        [[nodiscard]] NetworkRegion clone() const
        {
            return { *this };
        }


        [[nodiscard]] std::string toString() const;


        // Getters.
        [[nodiscard]] std::vector<region::Region> const &getRegions() const { return regions; }
        [[nodiscard]] absl::btree_set<int> &getModifiableIsAOrC() { return isAorC; }
        [[nodiscard]] std::deque<absl::btree_map<int, boost::dynamic_bitset<>>> &getModifiableClockOrdering() { return clockOrdering; }


        // Setters.
        void setRegionGivenIndex(const int idx, const region::Region &reg) { regions[idx] = reg; }


        NetworkRegion &operator=(const NetworkRegion &other)
        {
            if (this != &other)
            {
                regions = other.regions;
                isAorC = other.isAorC;
                clockOrdering = other.clockOrdering;
            }
            return *this;
        }


        bool operator==(const NetworkRegion &other) const
        {
            if (this == &other)
                return true;
            if (regions != other.regions)
                return false;
            if (isAorC != other.isAorC)
                return false;
            if (clockOrdering != other.clockOrdering)
                return false;

            return true;
        }


        bool operator!=(const NetworkRegion &other) const
        {
            return !(*this == other);
        }


        friend struct NetworkRegionHash;
    };


    /// Hash function for NetworkRegion.
    struct NetworkRegionHash
    {
        std::size_t operator()(const NetworkRegion &networkRegion) const
        {
            std::size_t seed = 0;

            // Hashing regions.
            constexpr region::RegionHash regionHash;
            for (const auto &region: networkRegion.getRegions())
                hash_combine(seed, regionHash(region));

            // Hashing isAorC.
            // The set must preserve the order of its elements for the hash to work.
            hash_combine(seed, networkRegion.isAorC.size());
            for (int val: networkRegion.isAorC)
                hash_combine(seed, val);

            // Hashing clockOrdering.
            hash_combine(seed, networkRegion.clockOrdering.size());
            for (const auto &map: networkRegion.clockOrdering)
            {
                // The map must preserve the order of its elements for the hash to work.
                hash_combine(seed, map.size());
                for (const auto &[key, bitset]: map)
                {
                    hash_combine(seed, key);
                    hash_combine(seed, hash_bitset(bitset));
                }
            }

            return seed;
        }
    };
}

#endif //TARZAN_NETWORKREGION_H
