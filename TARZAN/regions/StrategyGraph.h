#ifndef TARZAN_STRATEGYGRAPH_H
#define TARZAN_STRATEGYGRAPH_H

#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/printing_utilities.h"
#include <iostream>

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
    protected:
        // Regions used in comparisons (e.g., the goal regions in a reachability TCG).
        regionSet targetRegions{};

        // Regions corresponding to the starting states of the strategy graph (i.e., those corresponding to initial regions in the original RTS).
        std::vector<Region> heads{};

        // An adjacency list from a source region to a target region (under the move semantics of TCG) with additional information to synthesize strategies.
        std::unordered_map<Region, strategyTransitionSet, RegionHash> strategyTransitions{};


        /**
         * @brief Builds a DOT node label for a region.
         *
         * @param reg the region to label.
         * @param labelId a string o append near to the location name.
         * @param intToLocations a map from location indices to location names.
         * @param indicesToClocks a map from clock indices to clock names.
         * @param locationsToPlayers a map from location indices to players.
         * @return the DOT label string.
         */
        [[nodiscard]] static std::string dotRegionLabel(const Region &reg,
                                                        const std::string &labelId,
                                                        const std::unordered_map<int, std::string> &intToLocations,
                                                        const std::unordered_map<int, std::string> &indicesToClocks,
                                                        const absl::flat_hash_map<int, players_sym> &locationsToPlayers);


        /**
         * @brief Builds a DOT node label for a region.
         *
         * @param reg the region to label.
         * @param intToLocations a map from location indices to location names.
         * @param indicesToClocks a map from clock indices to clock names.
         * @param locationsToPlayers a map from location indices to players.
         * @return the DOT label string.
         */
        [[nodiscard]] static std::string dotRegionLabel(const Region &reg,
                                                        const std::unordered_map<int, std::string> &intToLocations,
                                                        const std::unordered_map<int, std::string> &indicesToClocks,
                                                        const absl::flat_hash_map<int, players_sym> &locationsToPlayers);


        /**
         * @brief Builds a DOT edge label for a strategy transition.
         *
         * @param arenaTransition the arena transition associated with the strategy transition.
         * @param moveCV the clock valuation at the moment the discrete transition is taken.
         * @param indicesToClocks a map from clock indices to clock names.
         * @return the DOT label string.
         */
        [[nodiscard]] static std::string dotEdgeLabel(const transition &arenaTransition,
                                                      const clockValuation &moveCV,
                                                      const std::unordered_map<int, std::string> &indicesToClocks);


    private:
        /**
         * @brief Prints a clock valuation with clock names during strategy synthesis.
         *
         * @param cv the clock valuation to print.
         * @param indicesToClocks a map from clock indices to clock names.
         * @param indent the indentation string to prepend to each line.
         */
        // TODO: if desired, this function can be modified to show the intervals in which clock values fall.
        inline static void printClockValuationInStrategy(const std::vector<std::pair<int, bool>> &cv,
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
        inline static void printRegionInStrategy(const Region &reg,
                                                 const std::unordered_map<int, std::string> &intToLocations,
                                                 const std::unordered_map<int, std::string> &indicesToClocks,
                                                 const absl::flat_hash_map<int, players_sym> &locationsToPlayers,
                                                 const std::string &indent);


        /**
         * @brief Escapes characters that would break a DOT double-quoted string label.
         *
         * @param s the string to escape.
         * @return the escaped string.
         */
        [[nodiscard]] inline static std::string escapeDot(const std::string &s);


        /**
         * @brief Formats a clock valuation as a human-readable string.
         *
         * @param cv the clock valuation to format.
         * @param indicesToClocks a map from clock indices to clock names.
         * @param separator the string inserted between consecutive clock entries.
         * @return the formatted clock valuation string.
         */
        [[nodiscard]] inline static std::string formatClockValuation(const clockValuation &cv,
                                                                     const std::unordered_map<int, std::string> &indicesToClocks,
                                                                     const std::string &separator);


    public:
        virtual ~StrategyGraph() = default;


        /**
         * @brief Adds a new transition to the 'transitions' map, possibly inserting a new key if not present.
         *
         * @param source the source region of the strategy transition.
         * @param arenaTransition a transition of the original Timed Arena over which the current strategy transition is computed.
         * @param target the target of the strategy transition.
         * @param cv the clock valuation of a discrete predecessor of target.
         */
        virtual void addStrategyTransition(const Region &source, const transition &arenaTransition, const Region &target, const clockValuation &cv);


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
        static void printRegionWithBox(int step,
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


        /**
         * @brief Deletes unnecessary strategy transitions from strategyTransitions.
         *
         * A strategy transition is unnecessary if its source is not contained in the intersectionSet parameter.
         *
         * @param intersectionSet the set of regions from which transitions in strategyTransitions must originate.
         */
        virtual void eraseUnnecessaryTransitions(const regionSet &intersectionSet)
        {
            std::erase_if(strategyTransitions, [&intersectionSet](const auto &pair) { return !intersectionSet.contains(pair.first); });
        }


        /**
         * @brief Converts the strategy graph to a .dot representation.
         *
         * @param path the path in which to save the .dot representation (must end with file_name.dot).
         * @param indicesToClocks a map from clock indices to clock names.
         * @param intToLocations a map from location indices to location names.
         * @param locationsToPlayers a map from location indices to players.
         *
         * @throws std::runtime_error if it fails to open the given .dot file path.
         */
        virtual void to_dot(const std::string &path,
                            const std::unordered_map<int, std::string> &indicesToClocks,
                            const std::unordered_map<int, std::string> &intToLocations,
                            const absl::flat_hash_map<int, players_sym> &locationsToPlayers);


        // To be overridden in StrategyGraphForConjunction.
        virtual void addNewStrategyTransitionMapToBack()
        {
            throw std::logic_error("Not supported by this StrategyGraph type.");
        }


        // To be overridden in StrategyGraphForConjunction.
        [[nodiscard]] virtual strategyTransitionSet getStrategyTransitionsGivenSourceAndIndex(const Region &source, const size_t index) const
        {
            std::cout << "Given source:\n" << source.toString() << "\n" << "Given index: " << index << std::endl;
            throw std::logic_error("Not supported by this StrategyGraph type.");
        }


        [[nodiscard]] virtual std::string to_string(const std::unordered_map<int, std::string> &intToLocations) const;
    };
}


#endif //TARZAN_STRATEGYGRAPH_H
