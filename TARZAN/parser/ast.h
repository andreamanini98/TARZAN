#ifndef AST_H
#define AST_H

#include "TARZAN/parser/comparison_op_enum.h"
#include "TARZAN/utilities/printing_utilities.h"

#include <vector>
#include <map>
#include <ranges>
#include <sstream>

// TODO: comment the code

// TODO: avoid code duplication.


namespace timed_automaton::ast
{
    struct clockConstraint
    {
        std::string clock;
        comparison_op constraintOperator;
        int comparingConstant;


        [[nodiscard]] std::string to_string() const
        {
            std::ostringstream oss;
            oss << clock << " " << constraintOperator << " " << comparingConstant;
            return oss.str();
        }
    };


    struct transition
    {
        std::string startingLocation;
        std::string action;
        std::vector<clockConstraint> clockGuard;
        std::vector<std::string> clocksToReset;
        std::string targetLocation;


        [[nodiscard]] std::string to_string() const
        {
            std::ostringstream oss;
            oss << "(" << startingLocation << ", " << action << ", "
                    << "[" << join_elements(clockGuard, " and ") << "], "
                    << "[" << join_elements(clocksToReset, ", ") << "], "
                    << targetLocation << ")";
            return oss.str();
        }
    };


    // Defining the map to hold Timed Automata locations.
    using loc_pair = std::pair<std::string, std::optional<bool>>;
    using loc_map = std::unordered_map<std::string, std::optional<bool>>;


    struct timedAutomaton
    {
        std::string name;
        std::vector<std::string> clocks;
        std::vector<std::string> actions;
        loc_map locations;
        std::vector<transition> transitions;


        /**
         * @return an int representing the maximum constant appearing in the Timed Automaton.
         */
        [[nodiscard]] int getMaxConstant() const
        {
            int maxConstant{};
            for (const transition &tr: transitions)
                for (const clockConstraint &cc: tr.clockGuard)
                    if (maxConstant < cc.comparingConstant)
                        maxConstant = cc.comparingConstant;
            return maxConstant;
        }


        /**
         * @return a std::unordered_map from location names to integers.
         *
         * @warning To be used at the beginning, right after parsing a Timed Automaton, since at the current time of development this is
         *          the only way we map locations to integers.
         */
        // TODO: if needed to reverse from this, you need to obtain a new std::unordered_map having integer keys and std::string values.
        [[nodiscard]] std::unordered_map<std::string, int> mapLocationsToInt() const
        {
            std::unordered_map<std::string, int> map;
            int idx{};

            for (const auto &loc: locations | std::views::keys)
            {
                map[loc] = idx;
                idx++;
            }

            return map;
        }


        /**
         * @brief Collects transitions exiting from locations, grouping and indexing them.
         *
         * Each location must be treated as an integer. This way, the transitions exiting from such location can be easily
         * accessed through indexing the returned std::vector.
         * For example, if a region r0 is in location q = 2, then we can access the vector at index 2 (recall that q starts from 0) to get the
         * outgoing transitions from r0.
         *
         * @param locToIntMap a mapping from locations (represented by std::string) to int.
         * @return a std::vector of std::vector, where each inner vector contains the outgoing transitions from the location corresponding to an index.
         */
        // TODO: for the predecessors, you may also want to do the same but with incoming transitions instead of outgoing transitions.
        // TODO: maybe it can be done also by grouping indices instead of transitions (the indices correspond to the positions in the transitions
        //       std::vector), but let's first try with this.
        [[nodiscard]] std::vector<std::vector<transition>> getOutTransitions(const std::unordered_map<std::string, int> &locToIntMap) const
        {
            std::vector<std::vector<transition>> outTransitions;
            outTransitions.resize(locToIntMap.size());

            for (const auto &t: transitions)
            {
                const int idx = locToIntMap.at(t.startingLocation);
                outTransitions[idx].push_back(t);
            }

            return outTransitions;
        }


        [[nodiscard]] std::string to_string() const
        {
            std::ostringstream oss;
            oss << "Timed Automaton " << name << std::endl;
            oss << "Clocks:\n" << join_elements(clocks, ", ") << std::endl;
            oss << "Actions:\n" << join_elements(actions, ", ") << std::endl;
            oss << "Locations:\n";
            for (const auto &[key, value]: locations)
            {
                oss << key << ", ";
                if (value.has_value())
                    oss << (value.value() ? "true" : "false");
                else
                    oss << "null_opt";
                oss << std::endl;
            }
            oss << "Transitions:\n" << join_elements(transitions, "\n") << std::endl;
            return oss.str();
        }
    };


    // Defining the map to hold Timed Arenas locations.
    using arena_loc = std::pair<char, std::optional<bool>>;
    using arena_loc_pair = std::pair<std::string, arena_loc>;
    using arena_loc_map = std::unordered_map<std::string, arena_loc>;


    struct timedArena
    {
        std::string name;
        std::vector<std::string> clocks;
        std::vector<std::string> actions;
        arena_loc_map locations;
        std::vector<transition> transitions;


        /**
         * @return an int representing the maximum constant appearing in the Timed Arena.
         */
        [[nodiscard]] int getMaxConstant() const
        {
            int maxConstant{};
            for (const transition &tr: transitions)
                for (const clockConstraint &cc: tr.clockGuard)
                    if (maxConstant < cc.comparingConstant)
                        maxConstant = cc.comparingConstant;
            return maxConstant;
        }


        /**
         * @return a std::unordered_map from location names to integers.
         *
         * @warning To be used at the beginning, right after parsing a Timed Automaton, since at the current time of development this is
         *          the only way we map locations to integers.
         */
        // TODO: if needed to reverse from this, you need to obtain a new std::unordered_map having integer keys and std::string values.
        [[nodiscard]] std::unordered_map<std::string, int> mapLocationsToInt() const
        {
            std::unordered_map<std::string, int> map;
            int idx{};

            for (const auto &loc: locations | std::views::keys)
            {
                map[loc] = idx;
                idx++;
            }

            return map;
        }


        /**
         * @brief Collects transitions exiting from locations, grouping and indexing them.
         *
         * Each location must be treated as an integer. This way, the transitions exiting from such location can be easily
         * accessed through indexing the returned std::vector.
         * For example, if a region r0 is in location q = 2, then we can access the vector at index 2 (recall that q starts from 0) to get the
         * outgoing transitions from r0.
         *
         * @param locToIntMap a mapping from locations (represented by std::string) to int.
         * @return a std::vector of std::vector, where each inner vector contains the outgoing transitions from the location corresponding to an index.
         */
        // TODO: for the predecessors, you may also want to do the same but with incoming transitions instead of outgoing transitions.
        // TODO: maybe it can be done also by grouping indices instead of transitions (the indices correspond to the positions in the transitions
        //       std::vector), but let's first try with this.
        [[nodiscard]] std::vector<std::vector<transition>> getOutTransitions(const std::unordered_map<std::string, int> &locToIntMap) const
        {
            std::vector<std::vector<transition>> outTransitions;
            outTransitions.resize(locToIntMap.size());

            for (const auto &t: transitions)
            {
                const int idx = locToIntMap.at(t.startingLocation);
                outTransitions[idx].push_back(t);
            }

            return outTransitions;
        }


        [[nodiscard]] std::string to_string() const
        {
            std::ostringstream oss;
            oss << "Timed Arena " << name << std::endl;
            oss << "Clocks:\n" << join_elements(clocks, ", ") << std::endl;
            oss << "Actions:\n" << join_elements(actions, ", ") << std::endl;
            oss << "Locations:\n";
            for (const auto &[location_name, location_info]: locations)
            {
                const auto &[player, initial] = location_info;
                oss << location_name << ", <" << player << ", ";
                if (initial.has_value())
                    oss << (initial.value() ? "true" : "false");
                else
                    oss << "null_opt";
                oss << ">\n";
            }
            oss << "Transitions:\n" << join_elements(transitions, "\n") << std::endl;
            return oss.str();
        }
    };
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::clockConstraint &g)
{
    return os << g.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::transition &t)
{
    return os << t.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::timedAutomaton &t)
{
    return os << t.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::timedArena &t)
{
    return os << t.to_string();
}

#endif //AST_H
