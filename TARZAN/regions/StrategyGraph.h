#ifndef TARZAN_STRATEGYGRAPH_H
#define TARZAN_STRATEGYGRAPH_H

#include "TARZAN/regions/Region.h"

using regionSet = std::unordered_set<region::Region, region::RegionHash>;


/// Defining a hash for the strategy transitions.
struct ActionRegionHash
{
    std::size_t operator()(const std::pair<std::string, region::Region> &p) const
    {
        std::size_t seed = std::hash<std::string>{}(p.first);
        hash_combine(seed, region::RegionHash{}(p.second));
        return seed;
    }
};

using transitionSet = std::unordered_set<std::pair<std::string, region::Region>, ActionRegionHash>;


namespace region
{
    class StrategyGraph
    {
        // Regions corresponding to the starting points of the strategy graph (i.e., those corresponding to initial regions in the original RTS).
        std::vector<Region> heads{};

        // Regions used in comparisons (e.g., the goal regions in a reachability TCG).
        regionSet targetRegions{};

        // An adjacency list from a source region to a target region (under the move semantics of TCG).
        // The value of this map is a vector of pairs, where the string is an action name and the region is the target of such a move.
        std::unordered_map<Region, transitionSet, RegionHash> transitions{};


    public:
        /**
         * Adds a new transition to the 'transitions' map, possibly inserting a new key if not present.
         *
         * @param sourceRegion the source region of the strategy transition.
         * @param action an action corresponding to the controller choice.
         * @param targetRegion the target of the strategy transition.
         */
        void addStrategyTransition(const Region &sourceRegion, const std::string &action, const Region &targetRegion);


        /**
         * Returns all strategy transitions associated with the given sourceRegion.
         *
         * @param sourceRegion the desired source region.
         * @return all strategy transitions associated with sourceRegion, or an empty vector if none exist.
         */
        [[nodiscard]] transitionSet getTransitionsGivenSource(const Region &sourceRegion) const;


        // Getters.
        [[nodiscard]] std::vector<Region> getHeads() const { return heads; }
        [[nodiscard]] regionSet getTargetRegions() const { return targetRegions; }
        [[nodiscard]] std::unordered_map<Region, transitionSet, RegionHash> getTransitions() const { return transitions; }


        // Setters.
        void setHeads(const std::vector<Region> &newHeads) { this->heads = newHeads; }
        void setTargetRegions(const regionSet &newTargetRegions) { targetRegions = newTargetRegions; }


        [[nodiscard]] std::string to_string() const;


        [[nodiscard]] std::string to_string(const std::unordered_map<int, std::string> &intToLocations) const;
    };
}


#endif //TARZAN_STRATEGYGRAPH_H
