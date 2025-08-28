#ifndef AST_H
#define AST_H

#include "TARZAN/parser/comparison_op_enum.h"
#include "TARZAN/utilities/printing_utilities.h"

#include <vector>
#include <map>
#include <ranges>

// TODO: comment the code

// TODO: avoid code duplication.


namespace timed_automaton::ast
{
    struct clockConstraint
    {
        std::string clock;
        comparison_op constraintOperator;
        int comparingConstant;


        /**
         * @brief Computes the satisfiability of a clock constraint.
         *
         * @param clockValue the integer value of the constraint's clock.
         * @param isFractionalPartGreaterThanZero true if the clock has a fractional part that is greater than zero, false otherwise.
         * @return true if the constraint is satisfied, false otherwise.
         */
        [[nodiscard]] bool isSatisfied(int clockValue, bool isFractionalPartGreaterThanZero) const;


        [[nodiscard]] std::string getClockName() const { return clock; }


        [[nodiscard]] std::string to_string() const;
    };


    struct transition
    {
        std::string startingLocation;
        std::string action;
        std::vector<clockConstraint> clockGuard;
        std::vector<std::string> clocksToReset;
        std::string targetLocation;


        /**
         * @brief Computes the satisfiability of a transition's guard.
         *
         * @param clockValuation the current clock valuation (integer values and a boolean denoting whether the fractional part is greater than zero).
         * @param clocksIndices the indices of the clocks as they appear in the clocks vector of a Timed Automaton.
         * @return true if the guard is satisfied, false otherwise.
         *
         * @attention Works only if the guard is a conjunction of clock constraints, where a clock constraint is (x ~ c), with ~ in {<, <=, =, >=, >}.
         */
        [[nodiscard]] bool isGuardSatisfied(const std::vector<std::pair<int, bool>> &clockValuation,
                                            const std::unordered_map<std::string, int> &clocksIndices) const;


        [[nodiscard]] std::string to_string() const;
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
        [[nodiscard]] int getMaxConstant() const;


        /**
         * @return a std::unordered_map from clock names to their index in the clocks vector.
         *
         * @warning To be used at the beginning, right after parsing a Timed Automaton, since at the current time of development this is
         *          the only way we map clocks (saved as std::string) to integers.
         */
        [[nodiscard]] std::unordered_map<std::string, int> getClocksIndices() const;


        /**
         * @return a std::unordered_map from location names to integers.
         *
         * @warning To be used at the beginning, right after parsing a Timed Automaton, since at the current time of development this is
         *          the only way we map locations to integers.
         */
        // TODO: if needed to reverse from this, you need to obtain a new std::unordered_map having integer keys and std::string values.
        [[nodiscard]] std::unordered_map<std::string, int> mapLocationsToInt() const;


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
        // TODO: for the discrete predecessors, you may also want to do the same but with incoming transitions instead of outgoing transitions.
        // TODO: maybe it can be done also by grouping indices instead of transitions (the indices correspond to the positions in the transitions
        //       std::vector), but let's first try with this.
        [[nodiscard]] std::vector<std::vector<transition>> getOutTransitions(const std::unordered_map<std::string, int> &locToIntMap) const;


        [[nodiscard]] std::string to_string() const;
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
        [[nodiscard]] int getMaxConstant() const;


        /**
         * @return a std::unordered_map from clock names to their index in the clocks vector.
         *
         * @warning To be used at the beginning, right after parsing a Timed Arena, since at the current time of development this is
         *          the only way we map clocks (saved as std::string) to integers.
         */
        [[nodiscard]] std::unordered_map<std::string, int> getClocksIndices() const;


        /**
         * @return a std::unordered_map from location names to integers.
         *
         * @warning To be used at the beginning, right after parsing a Timed Arena, since at the current time of development this is
         *          the only way we map locations to integers.
         */
        // TODO: if needed to reverse from this, you need to obtain a new std::unordered_map having integer keys and std::string values.
        [[nodiscard]] std::unordered_map<std::string, int> mapLocationsToInt() const;


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
        // TODO: for the discrete predecessors, you may also want to do the same but with incoming transitions instead of outgoing transitions.
        // TODO: maybe it can be done also by grouping indices instead of transitions (the indices correspond to the positions in the transitions
        //       std::vector), but let's first try with this.
        [[nodiscard]] std::vector<std::vector<transition>> getOutTransitions(const std::unordered_map<std::string, int> &locToIntMap) const;


        [[nodiscard]] std::string to_string() const;
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
