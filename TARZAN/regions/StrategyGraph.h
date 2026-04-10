#ifndef TARZAN_STRATEGYGRAPH_H
#define TARZAN_STRATEGYGRAPH_H

#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/printing_utilities.h"

#define BOX_WIDTH 42

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


        /**
         * @brief Prints a clock valuation with clock names during strategy synthesis.
         *
         * @param cv the clock valuation to print.
         * @param indicesToClocks a map from clock indices to clock names.
         * @param indent the indentation string to prepend to each line.
         */
        // TODO: if desired, this function can be modified to show the intervals in which clock values fall.
        static inline void printClockValuationInStrategy(const std::vector<std::pair<int, bool>> &cv,
                                                         const std::unordered_map<int, std::string> &indicesToClocks,
                                                         const std::string &indent);


        /**
         * @brief Prints a region (location name and clock valuation).
         *
         * @param reg the region to print.
         * @param intToLocations a map from location indices to location names.
         * @param indicesToClocks a map from clock indices to clock names.
         * @param locationsToPlayers a map from location indices to players.
         * @param indent the indentation string to prepend to each line.
         */
        static inline void printRegionInStrategy(const Region &reg,
                                                 const std::unordered_map<int, std::string> &intToLocations,
                                                 const std::unordered_map<int, std::string> &indicesToClocks,
                                                 const absl::flat_hash_map<int, players_sym> &locationsToPlayers,
                                                 const std::string &indent);


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


        /**
         * @brief Prints a region bounded by a box for pretty printing a strategy.
         *
         * @param step the step counter, i.e., how many moves have been executed in the game.
         * @param reg the region to print.
         * @param intToLocations a map from location indices to location names.
         * @param indicesToClocks a map from clock indices to clock names.
         * @param locationsToPlayers a map from location indices to players.
         * @param symbolNextToStep a string to print next to the step counter.
         */
        static inline void printRegionWithBox(int step,
                                              const Region &reg,
                                              const std::unordered_map<int, std::string> &intToLocations,
                                              const std::unordered_map<int, std::string> &indicesToClocks,
                                              const absl::flat_hash_map<int, players_sym> &locationsToPlayers,
                                              const std::string &symbolNextToStep);


        /**
         * @brief Prints a transition of the strategy graph.
         *
         * @param step the step counter, i.e., how many moves have been executed in the game.
         * @param current the region to print.
         * @param intToLocations a map from location indices to location names.
         * @param indicesToClocks a map from clock indices to clock names.
         * @param locationsToPlayers a map from location indices to players.
         * @param moveClockValuation the clock valuation before a discrete transition is taken in a move.
         * @param arenaTransition a transition of the original Timed Arena over which the current strategy transition is computed.
         */
        static void printStrategyTransition(int step,
                                            const Region &current,
                                            const std::unordered_map<int, std::string> &intToLocations,
                                            const std::unordered_map<int, std::string> &indicesToClocks,
                                            const absl::flat_hash_map<int, players_sym> &locationsToPlayers,
                                            const clockValuation &moveClockValuation,
                                            const transition &arenaTransition);


        [[nodiscard]] std::string to_string(const std::unordered_map<int, std::string> &intToLocations) const;
    };
}


#endif //TARZAN_STRATEGYGRAPH_H
