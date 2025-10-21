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

        /// Map between integer variables and their value.
        absl::btree_map<std::string, int> networkVariables{};

        /**
         * Target locations for reachability checking, where the i-th element contains the target location for the i-th automaton,
         * or std::nullopt if no target is specified for that automaton. This vector is permuted along with regions during canonicalization.
         *
         * @note This field does NOT participate in equality/hash comparisons as it's only used for reachability objectives.
         *
         * @warning Must be permuted along with regions in symmetry reduction.
         */
        // TODO: vedere se si riesce a togliere questo attributo da questa classe mantenendo sempre però gli ordinamenti coerenti nella symmetry reduction.
        std::vector<std::optional<int>> targetLocations{};

        /**
         * Goal clock constraints for reachability checking, where the i-th element contains clock constraints
         * that must be satisfied for the i-th automaton's clocks. This vector is permuted along with regions during canonicalization.
         *
         * @note This field does NOT participate in equality/hash comparisons as it's only used for reachability objectives.
         *
         * @warning Must be permuted along with regions in symmetry reduction.
         */
        // TODO: vedere se si riesce a togliere questo attributo da questa classe mantenendo sempre però gli ordinamenti coerenti nella symmetry reduction.
        std::vector<std::vector<timed_automaton::ast::clockConstraint>> goalClockConstraints{};


    public:
        NetworkRegion() = default;


        NetworkRegion(const std::vector<region::Region> &regions,
                      const absl::btree_map<std::string, int> &networkVariables,
                      const bool allRegionsAreInitial) : regions(regions), networkVariables(networkVariables)
        {
            // If every region is initial, they must all belong to class A.
            if (allRegionsAreInitial)
            {
                const int numRegions = static_cast<int>(regions.size());

                // We insert values from 0 to numRegions - 1 since every region is initial.
                // We do not insert the index of regions with no clocks, since the computation of delay successors relies on the content of isAorC.
                for (int i = 0; i < numRegions; i++)
                    if (regions[i].getNumberOfClocks() > 0)
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
        [[nodiscard]] NetworkRegion getImmediateDelaySuccessor(const std::vector<std::vector<int>> &maxConstants) const;


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
            const std::vector<std::reference_wrapper<const std::vector<transition>>> &transitions,
            const std::vector<std::unordered_map<std::string, int>> &clockIndices,
            const std::vector<std::unordered_map<std::string, int>> &locationsToInt) const;


        /**
         * @brief Computes the canonical form of this network region by sorting symmetric processes to the lexicographically smallest permutation.
         *
         * @param symmetryGroups groups of automaton indices that are symmetric. Each inner vector contains indices of automata that are structurally identical.
         * @return canonical NetworkRegion representing the equivalence class.
         */
        [[nodiscard]] NetworkRegion getCanonicalForm(const std::vector<std::vector<int>> &symmetryGroups) const;


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
        [[nodiscard]] std::vector<region::Region> &getModifiableRegions() { return regions; }
        [[nodiscard]] absl::btree_set<int> &getModifiableIsAOrC() { return isAorC; }
        [[nodiscard]] std::deque<absl::btree_map<int, boost::dynamic_bitset<>>> &getModifiableClockOrdering() { return clockOrdering; }
        [[nodiscard]] absl::btree_map<std::string, int> &getModifiableNetworkVariables() { return networkVariables; }
        [[nodiscard]] absl::btree_map<std::string, int> const &getNetworkVariables() const { return networkVariables; }
        [[nodiscard]] std::vector<std::optional<int>> const &getTargetLocations() const { return targetLocations; }
        [[nodiscard]] std::vector<std::optional<int>> &getModifiableTargetLocations() { return targetLocations; }
        [[nodiscard]] std::vector<std::vector<timed_automaton::ast::clockConstraint>> const &getGoalClockConstraints() const { return goalClockConstraints; }
        [[nodiscard]] std::vector<std::vector<timed_automaton::ast::clockConstraint>> &getModifiableGoalClockConstraints() { return goalClockConstraints; }


        // Setters.
        void setRegionGivenIndex(const int idx, const region::Region &reg) { regions[idx] = reg; }
        void setNetworkVariables(const absl::btree_map<std::string, int> &networkVariables_p) { this->networkVariables = networkVariables_p; }
        void setTargetLocations(const std::vector<std::optional<int>> &targetLocations_p) { this->targetLocations = targetLocations_p; }


        void setGoalClockConstraints(const std::vector<std::vector<timed_automaton::ast::clockConstraint>> &goalClockConstraints_p)
        {
            this->goalClockConstraints = goalClockConstraints_p;
        }


        NetworkRegion &operator=(const NetworkRegion &other) = default;


        bool operator==(const NetworkRegion &other) const
        {
            return isAorC == other.isAorC &&
                   networkVariables == other.networkVariables &&
                   clockOrdering == other.clockOrdering &&
                   regions == other.regions;
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

            // Hashing the integer variables.
            hash_combine(seed, networkRegion.networkVariables.size());
            for (const auto &[varName, varValue]: networkRegion.networkVariables)
            {
                hash_combine(seed, varName);
                hash_combine(seed, varValue);
            }

            return seed;
        }
    };
}

#endif //TARZAN_NETWORKREGION_H
