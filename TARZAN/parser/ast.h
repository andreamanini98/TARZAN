#ifndef AST_H
#define AST_H

#include "enums/comparison_op_enum.h"
#include "enums/input_output_action_enum.h"

#include <vector>
#include <map>
#include <ranges>
#include "absl/container/flat_hash_map.h"

// TODO: avoid code duplication (if possible).


// The following is the grammar for the Liana DSL used to create Timed Automata (STILL NEED TO DEFINE THE TIMED ARENA ONE).
// Whether the actions are input or output actions must be specified only in the transitions.
// Up to now, it is better to specify clock constraints for clocks that should not appear in a guard as x >= 0.
// T and F are syntax sugar for true and false.
//
//  <automaton> -> 'create' 'automaton' <literal>
//                 '{'
//                 'clocks'      '{' <clocks_rule> '}'
//                 'actions'     '{' <literal> (, <literal>)* '}'
//                 'locations'   '{' <locations_rule> '}'
//                 'transitions' '{' <transition_rule> (, <transition_rule>)* ';' '}'
//                 '}'
//
//  <clocks_rule> -> <literal> (, <literal>)* ';'
//
//  <locations_rule> -> <loc_rule> (, <loc_rule>)* ';'
//
//  <loc_rule> -> <literal> <loc_content_rule>
//
//  <loc_content_rule> -> '<' (eps | 'ini' ':' <bool> (eps | ',' 'inv' ':' <guard_rule>) | 'inv' ':' <guard_rule>) '>'
//
//  <bool> -> 'T' | 'F' | 'true' | 'false'
//
//  <transition_rule> -> '('
//                       <literal> ','
//                       <actions_rule> ','
//                       <guard_rule> ','
//                       '[' (eps | <literal> (',' <literal>)*) ']' ','
//                       <literal>
//                       ')'
//
//  <guard_rule> -> '[' <clock_constraint_rule> (, <clock_constraint_rule>)* ']'
//
//  <actions_rule> -> <literal> (eps | <input_output_action>)
//
//  <input_output_action> -> '!' | '?'
//
//  <clock_constraint_rule> -> '(' <literal> ',' <comparison_operator> ',' <int> ')'
//
//  <comparison_operator> -> '<' | '<=' | '=' | '>=' | '>'
//
//  <int> -> '1..9'('0..9')*
//
//  <literal> -> ('a..z' | 'A..Z' | '0..9' | '_' )+


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


    struct locationContent
    {
        bool isInitial;
        std::vector<clockConstraint> invariant;


        [[nodiscard]] std::string to_string() const;
    };


    // Defining an action such that it can also handle the case of being an input or an output action in a network of TA.
    using act = std::pair<std::string, std::optional<in_out_act>>;


    struct transition
    {
        std::string startingLocation;
        act action;
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
    using loc_pair = std::pair<std::string, locationContent>;
    using loc_map = std::unordered_map<std::string, locationContent>;


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
         * @brief Returns the int indices of the initial locations given a mapping from location names to int values.
         *
         * @param locToIntMap the mapping from location names to int values.
         * @return a std::vector containing the indices of the initial locations.
         */
        [[nodiscard]] std::vector<int> getInitialLocations(const std::unordered_map<std::string, int> &locToIntMap) const;


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
        [[nodiscard]] std::vector<std::vector<transition>> getOutTransitions(const std::unordered_map<std::string, int> &locToIntMap) const;


        /**
         * @brief Collects transitions entering into locations, grouping and indexing them.
         *
         * Each location must be treated as an integer. This way, the transitions entering such a location can be easily
         * accessed through indexing the returned std::vector.
         * For example, if a region r0 is in location q = 2, then we can access the vector at index 2 (recall that q starts from 0) to get the
         * outgoing transitions from r0.
         *
         * @param locToIntMap a mapping from locations (represented by std::string) to int.
         * @return a std::vector of std::vector, where each inner vector contains the ingoing transitions from the location corresponding to an index.
         */
        [[nodiscard]] std::vector<std::vector<transition>> getInTransitions(const std::unordered_map<std::string, int> &locToIntMap) const;


        /**
         * @brief Collects the invariants of the Timed Automaton.
         *
         * @param locToIntMap a mapping from locations (represented by std::string) to int.
         * @return a map from integers representing locations to invariants of each location.
         */
        [[nodiscard]] absl::flat_hash_map<int, std::vector<clockConstraint>> getInvariants(
            const std::unordered_map<std::string, int> &locToIntMap) const;


        [[nodiscard]] std::string to_string() const;
    };


    // Defining the map to hold Timed Arenas locations.
    using arena_loc = std::pair<char, locationContent>;
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
         * @brief Returns the int indices of the initial locations given a mapping from location names to int values.
         *
         * @param locToIntMap the mapping from location names to int values.
         * @return a std::vector containing the indices of the initial locations.
         */
        [[nodiscard]] std::vector<int> getInitialLocations(const std::unordered_map<std::string, int> &locToIntMap) const;


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
        [[nodiscard]] std::vector<std::vector<transition>> getOutTransitions(const std::unordered_map<std::string, int> &locToIntMap) const;


        /**
         * @brief Collects transitions entering into locations, grouping and indexing them.
         *
         * Each location must be treated as an integer. This way, the transitions entering such a location can be easily
         * accessed through indexing the returned std::vector.
         * For example, if a region r0 is in location q = 2, then we can access the vector at index 2 (recall that q starts from 0) to get the
         * outgoing transitions from r0.
         *
         * @param locToIntMap a mapping from locations (represented by std::string) to int.
         * @return a std::vector of std::vector, where each inner vector contains the ingoing transitions from the location corresponding to an index.
         */
        [[nodiscard]] std::vector<std::vector<transition>> getInTransitions(const std::unordered_map<std::string, int> &locToIntMap) const;


        /**
         * @brief Collects the invariants of the Timed Automaton.
         *
         * @param locToIntMap a mapping from locations (represented by std::string) to int.
         * @return a map from integers representing locations to invariants of each location.
         */
        [[nodiscard]] absl::flat_hash_map<int, std::vector<clockConstraint>> getInvariants(
            const std::unordered_map<std::string, int> &locToIntMap) const;


        [[nodiscard]] std::string to_string() const;
    };
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::clockConstraint &g)
{
    return os << g.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::locationContent &l)
{
    return os << l.to_string();
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
