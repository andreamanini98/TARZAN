#ifndef TARZAN_STRATEGYGRAPHFORCONJUNCTION_H
#define TARZAN_STRATEGYGRAPHFORCONJUNCTION_H

#include "StrategyGraph.h"


namespace region
{
    class StrategyGraphForConjunction : public StrategyGraph
    {
        // A vector of strategy transitions, useful to follow such transitions step by step during strategy synthesis.
        std::vector<std::unordered_map<Region, strategyTransitionSet, RegionHash>> strategyTransitionsForConjunction{};


    public:
        /**
         * @brief Adds a new transition to the map in the back position of strategyTransitionsForConjunction, possibly inserting a new key if not present.
         *
         * @param source the source region of the strategy transition.
         * @param arenaTransition a transition of the original Timed Arena over which the current strategy transition is computed.
         * @param target the target of the strategy transition.
         * @param cv the clock valuation of a discrete predecessor of target.
         *
         * @throws std::logic_error if strategyTransitionsForConjunction is empty.
         */
        void addStrategyTransition(const Region &source,
                                   const transition &arenaTransition,
                                   const Region &target,
                                   const clockValuation &cv) override;


        /**
         * @brief Adds a new empty map to the back of strategyTransitionsForConjunction. Useful to differentiate between different steps of the strategy.
         */
        void addNewStrategyTransitionMapToBack() override;


        /**
         * @brief Returns all strategy transitions associated with the given source, taking them from the back of strategyTransitionsForConjunction.
         *
         * @param source the desired source region.
         * @param index the index of strategyTransitionsForConjunction from which to return strategy transitions.
         * @return all strategy transitions associated with source at position index in strategyTransitionsForConjunction, or an empty vector if none exist.
         *
         * @throws std::logic_error if the index is too big.
         */
        [[nodiscard]] strategyTransitionSet getStrategyTransitionsGivenSourceAndIndex(const Region &source, size_t index) const override;


        /**
         * @brief Deletes unnecessary strategy transitions from the unordered set in the back of strategyTransitionsForConjunction.
         *
         * A strategy transition is unnecessary if its source is not contained in the intersectionSet parameter.
         *
         * @param intersectionSet the set of regions from which transitions in strategyTransitions must originate.
         *
         * @throws std::logic_error if strategyTransitionsForConjunction is empty.
         */
        void eraseUnnecessaryTransitions(const regionSet &intersectionSet) override
        {
            if (strategyTransitionsForConjunction.empty())
                throw std::logic_error("strategyTransitionsForConjunction is empty!");

            std::erase_if(strategyTransitionsForConjunction.back(), [&intersectionSet](const auto &pair) { return !intersectionSet.contains(pair.first); });
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
        void to_dot(const std::string &path,
                    const std::unordered_map<int, std::string> &indicesToClocks,
                    const std::unordered_map<int, std::string> &intToLocations,
                    const absl::flat_hash_map<int, players_sym> &locationsToPlayers) override;


        // Getters.
        [[nodiscard]] std::vector<std::unordered_map<Region, strategyTransitionSet, RegionHash>> const &getStrategyTransitionsForConjunction() const
        {
            return strategyTransitionsForConjunction;
        }


        [[nodiscard]] std::string to_string(const std::unordered_map<int, std::string> &intToLocations) const override;
    };
}

#endif //TARZAN_STRATEGYGRAPHFORCONJUNCTION_H
