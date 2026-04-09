#ifndef TARZAN_STRATEGYGRAPH_H
#define TARZAN_STRATEGYGRAPH_H

#include "TARZAN/regions/Region.h"

using regionSet = std::unordered_set<region::Region, region::RegionHash>;


/// Defining a hash for the strategy transitions.
struct ActionRegionHash
{
    std::size_t operator()(const std::tuple<transition, region::Region, clockValuation> &t) const
    {
        std::size_t seed = 0;

        // For now, we do not hash the transitions. The following line can be uncommented to also hash them.
        // seed = timed_automaton::ast::transitionHash{}(std::get<0>(t));

        seed = region::RegionHash{}(std::get<1>(t));
        for (const auto &[intVal, hasFrac]: std::get<2>(t))
        {
            hash_combine(seed, intVal);
            hash_combine(seed, hasFrac);
        }
        return seed;
    }
};

// This is a set of tuples, where the elements of each tuple are described as follows:
// - transition: the original transition of the Timed Arena. Useful to retrieve actions, clock constraints, clock resets, and other useful information.
// - region::Region: the target region of the strategy transition. Useful to retrieve its location, clock valuation, and to perform checks on regions.
// - clockValuation: the clock valuation of the discrete predecessor of the region represented by region::Region.
//                   Useful to know the values of clocks prior to the strategy transition.
using strategyTransitionSet = std::unordered_set<std::tuple<transition, region::Region, clockValuation>, ActionRegionHash>;


namespace region
{
    class StrategyGraph
    {
        // Regions corresponding to the starting states of the strategy graph (i.e., those corresponding to initial regions in the original RTS).
        std::vector<Region> heads{};

        // Regions used in comparisons (e.g., the goal regions in a reachability TCG).
        regionSet targetRegions{};

        // An adjacency list from a source region to a target region (under the move semantics of TCG) with additional information to synthesize strategies.
        std::unordered_map<Region, strategyTransitionSet, RegionHash> strategyTransitions{};


    public:
        /**
         * @brief Adds a new transition to the 'transitions' map, possibly inserting a new key if not present.
         *
         * @param source the source region of the strategy transition.
         * @param arenaTransition a transition of the original Timed Arena over which the current strategy transition is computed.
         * @param target the target of the strategy transition.
         * @param cv the clock valuation of a discrete predecessor of target.
         */
        void addStrategyTransition(const Region &source, const transition &arenaTransition, const Region &target, const clockValuation &cv);


        /**
         * @brief Returns all strategy transitions associated with the given source.
         *
         * @param source the desired source region.
         * @return all strategy transitions associated with source, or an empty vector if none exist.
         */
        [[nodiscard]] strategyTransitionSet getStrategyTransitionsGivenSource(const Region &source) const;


        // Getters.
        [[nodiscard]] std::vector<Region> const &getHeads() const { return heads; }
        [[nodiscard]] regionSet const &getTargetRegions() const { return targetRegions; }
        [[nodiscard]] std::unordered_map<Region, strategyTransitionSet, RegionHash> const &getStrategyTransitions() const { return strategyTransitions; }


        // Setters.
        void setHeads(const std::vector<Region> &newHeads) { this->heads = newHeads; }
        void setTargetRegions(const regionSet &newTargetRegions) { this->targetRegions = newTargetRegions; }


        [[nodiscard]] std::string to_string(const std::unordered_map<int, std::string> &intToLocations) const;
    };
}


#endif //TARZAN_STRATEGYGRAPH_H
