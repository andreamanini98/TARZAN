#ifndef AST_H
#define AST_H

#include "enums/arithmetic_enum.h"
#include "enums/boolean_op_enum.h"
#include "enums/comparison_op_enum.h"
#include "enums/input_output_action_enum.h"

#include <vector>
#include <map>
#include <ranges>
#include <variant>
#include <absl/container/flat_hash_set.h>

#include "absl/container/flat_hash_map.h"
#include "absl/container/btree_map.h"
#include <boost/spirit/home/x3/support/ast/variant.hpp>

// TODO: avoid code duplication (if possible).


// The following is the grammar for the Liana DSL used to create Timed Automata.
// Whether the actions are input or output actions must be specified only in the transitions.
// T and F are syntax sugar for true and false.
// Integer variables in Timed Automata are automatically initialized to 0. To overcome this, a transition can be added to initialize them in the Timed Automaton itself.
//
//  <automaton> -> 'create' 'automaton' <literal>
//                 '{'
//                 'clocks'          '{' (eps | <literal> (, <literal>)* ';') '}'
//                 'actions'         '{' <literal> (, <literal>)* ';' '}'
//                 (eps | 'integers' '{' <literal> (, <literal>)* ';' '}')
//                 'locations'       '{' <locations_rule> '}'
//                 'transitions'     '{' <transition_rule> (, <transition_rule>)* ';' '}'
//                 '}'
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
//                       (eps | <boolean_expr> ',')
//                       '[' (eps | <literal> (',' <literal>)*) ']' ','
//                       (eps | '[' <assignment_expr> (',' <assignment_expr>)* ']' ',')
//                       <literal>
//                       ')'
//
//  <guard_rule> -> '[' (eps | <clock_constraint_rule> (, <clock_constraint_rule>)*) ']'
//
//  <actions_rule> -> <literal> (eps | <input_output_action>)
//
//  <input_output_action> -> '!' | '?'
//
//  <clock_constraint_rule> -> '(' <literal> ',' <comparison_operator> ',' <int> ')'
//
//  <comparison_operator> -> '<' | '<=' | '==' | '>=' | '>'
//
//  <int> -> '1..9'('0..9')*
//
//  <literal> -> ('a..z' | 'A..Z' | '0..9' | '_' )+
//
//
// The following is the grammar of expressions.
//
//  <assignment_expr> -> <variable> '=' <arithmetic_expr>
//
//  <arithmetic_expr> -> <additive_expr>
//
//  <additive_expr> -> <multiplicative_expr> (<add_op> <multiplicative_expr>)*
//
//  <multiplicative_expr> -> <primary_expr> (<mult_op> <primary_expr>)*
//
//  <primary_expr> -> <int> | <variable> | '(' <additive_expr> ')'
//
//  <variable> -> <literal>
//
//  <add_op> -> '+' | '-'
//
//  <mult_op> -> '*' | '/'
//
//  <boolean_expr> -> <boolean_or_expr>
//
//  <boolean_or_expr> -> <boolean_and_expr> (<or_op> <boolean_and_expr>)*
//
//  <boolean_and_expr> -> <boolean_term> (<and_op> <boolean_term>)*
//
//  <boolean_term> -> '(' <boolean_or_expr> ')' | <comparison_expr> | <bool>
//
//  <comparison_expr> <arithmetic_expr> <comparison_operator> <arithmetic_expr>
//
//  <or_op> -> '||'
//
//  <and_op> -> '&&'

// TODO: aggiustare liana con le locations urgenti.

// TODO: cambiare la chiave della mappa delle funzioni evaluate da stringa a intero (ti serve un mapping da stringhe ad interi come fai per i clock e le locations).

// TODO: implementa le locations urgenti
//       3) in RTSNetwork, se almeno una regione della rete è in una location urgente, non devi calcolare i delay ma solo i discrete
//          quando inizializzi una rete, crea un set di indici dove ogni indice rappresenta una regione che potrebbe avere una location urgente.
//          Poi controlla solo quelle e se la location corrente di una di qulle regioni è urgente skippi il calcolo dei delay successor.


// Reference examples for expression parser:
// https://wandbox.org/permlink/YlVEPhgKPNMiKADh
// https://www.boost.org/doc/libs/1_67_0/libs/spirit/example/x3/rexpr/rexpr_min/rexpr.cpp
namespace expr::ast
{
    /// A variable appearing in an expression. It only consists of its name.
    struct variable
    {
        std::string name;
    };


    // Forward declaring this struct so that it can be used in the structs below.
    struct binaryExpr;


    /// An arithmetic expression which can contain integers and variables.
    struct arithmeticExpr
    {
        // The forward_ast enables the recursive definition of this ast.
        std::variant<int, variable, boost::spirit::x3::forward_ast<binaryExpr>> value;


        // Implicit conversions, needed since we directly synthesize variables in the grammar.
        arithmeticExpr() = default;

        // NOLINTNEXTLINE(google-explicit-constructor)
        arithmeticExpr(int v) : value(v) {}

        // NOLINTNEXTLINE(google-explicit-constructor)
        arithmeticExpr(variable v) : value(std::move(v)) {}

        // NOLINTNEXTLINE(google-explicit-constructor)
        arithmeticExpr(boost::spirit::x3::forward_ast<binaryExpr> v) : value(std::move(v)) {}

        // NOLINTNEXTLINE(google-explicit-constructor)
        arithmeticExpr(binaryExpr const &v);

        // NOLINTNEXTLINE(google-explicit-constructor)
        arithmeticExpr(binaryExpr &&v);


        /**
         * @brief Evaluates an arithmetic expression given a variable context.
         *
         * @param variables a map from variables to their integer values.
         * @return the integer result of the evaluation.
         * @throws std::runtime_error if a variable is not found or division by zero occurs.
         */
        [[nodiscard]] int evaluate(const absl::btree_map<std::string, int> &variables) const;


        [[nodiscard]] std::string to_string() const;
    };


    /// A binary expression. We do not need to adapt this nor define a grammar for it, since we directly generate it with semantic actions.
    struct binaryExpr
    {
        arithmeticExpr left_expr;
        arithmetic_op op;
        arithmeticExpr right_expr;


        [[nodiscard]] std::string to_string() const;
    };


    // Implementing the deferred constructors after binaryExpr is complete.
    // Needed to break circular dependencies at compile time, as it happens, for example, when declaring a constructor of arithmeticExpr passing a binaryExpr.
    inline arithmeticExpr::arithmeticExpr(binaryExpr const &v) : value(boost::spirit::x3::forward_ast(v)) {}
    inline arithmeticExpr::arithmeticExpr(binaryExpr &&v) : value(boost::spirit::x3::forward_ast(std::move(v))) {}


    /// An assignment expression.
    struct assignmentExpr
    {
        variable lhs;
        arithmeticExpr rhs;


        /**
         * @brief Evaluates an assignment expression and updates the variable context.
         *
         * @param variables a map from variables to their integer values (is updated in the function).
         * @throws std::runtime_error if evaluation fails.
         */
        void evaluate(absl::btree_map<std::string, int> &variables) const;


        [[nodiscard]] std::string to_string() const;
    };


    /// A comparison expression between arithmetic expressions.
    struct comparisonExpr
    {
        arithmeticExpr left_expr;
        comparison_op op;
        arithmeticExpr right_expr;


        /**
         * @brief Evaluates a comparison expression.
         *
         * @param variables a map from variables to their integer values.
         * @return the result of the comparison.
         * @throws std::runtime_error if evaluation fails.
         */
        [[nodiscard]] bool evaluate(const absl::btree_map<std::string, int> &variables) const;


        [[nodiscard]] std::string to_string() const;
    };


    // Forward declaring this struct so that it can be used in the structs below.
    struct booleanBinaryExpr;


    /// A boolean expression which can contain booleans and comparison expressions.
    struct booleanExpr
    {
        std::variant<bool, comparisonExpr, boost::spirit::x3::forward_ast<booleanBinaryExpr>> value;


        // Implicit constructors
        booleanExpr() = default;

        // NOLINTNEXTLINE(google-explicit-constructor)
        booleanExpr(bool v) : value(v) {}

        // NOLINTNEXTLINE(google-explicit-constructor)
        booleanExpr(comparisonExpr v) : value(std::move(v)) {}

        // NOLINTNEXTLINE(google-explicit-constructor)
        booleanExpr(boost::spirit::x3::forward_ast<booleanBinaryExpr> v) : value(std::move(v)) {}

        // NOLINTNEXTLINE(google-explicit-constructor)
        booleanExpr(booleanBinaryExpr const &v);

        // NOLINTNEXTLINE(google-explicit-constructor)
        booleanExpr(booleanBinaryExpr &&v);


        /**
         * @brief Evaluates a boolean expression.
         *
         * @param variables a map from variables to their integer values.
         * @return the boolean value of the expression.
         * @throws std::runtime_error if evaluation fails.
         */
        [[nodiscard]] bool evaluate(const absl::btree_map<std::string, int> &variables) const;


        [[nodiscard]] std::string to_string() const;
    };


    /// A boolean binary expression.
    struct booleanBinaryExpr
    {
        booleanExpr left_expr;
        boolean_op op;
        booleanExpr right_expr;

        [[nodiscard]] std::string to_string() const;
    };


    // Implementing the deferred constructors after booleanBinaryExpr is complete.
    // Needed to break circular dependencies at compile time, as it happens, for example, when declaring a constructor of booleanExpr passing a booleanBinaryExpr.
    inline booleanExpr::booleanExpr(booleanBinaryExpr const &v) : value(boost::spirit::x3::forward_ast(v)) {}
    inline booleanExpr::booleanExpr(booleanBinaryExpr &&v) : value(boost::spirit::x3::forward_ast(std::move(v))) {}
}


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
        bool isUrgent;
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
        std::optional<expr::ast::booleanExpr> integerGuard;
        std::vector<std::string> clocksToReset;
        std::vector<expr::ast::assignmentExpr> integerAssignments;
        std::string targetLocation;


        /**
         * @brief Computes the satisfiability of a transition's guard.
         *
         * @param clockValuation the current clock valuation (integer values and a boolean denoting whether the fractional part is greater than zero).
         * @param clocksIndices the indices of the clocks as they appear in the clocks vector of a Timed Automaton.
         * @param variables a map from variables to their integer values (is updated in the function).
         * @return true if the guard is satisfied, false otherwise.
         *
         * @attention Works only if the guard is a conjunction of clock constraints, where a clock constraint is (x ~ c), with ~ in {<, <=, =, >=, >}.
         */
        [[nodiscard]] bool isTransitionSatisfied(const std::vector<std::pair<int, bool>> &clockValuation,
                                                 const std::unordered_map<std::string, int> &clocksIndices,
                                                 const absl::btree_map<std::string, int> &variables) const;


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
        std::vector<std::string> integerVariables;
        loc_map locations;
        std::vector<transition> transitions;


        /**
         * @return an int representing the maximum constant appearing in the Timed Automaton.
         */
        [[nodiscard]] int getMaxConstant() const;


        /**
         * @brief Computes the maximum constant appearing in a Timed Automaton for each clock.
         *
         * @param clocksIndices a map from clock names to their index in the clocks vector.
         * @return a vector containing in position i the maximum constant of the i-th clock, where the index i of the clock is given by clockIndices.
         */
        [[nodiscard]] std::vector<int> getMaxConstants(const std::unordered_map<std::string, int> &clocksIndices) const;


        /**
         * @return true if the Timed Automaton has at least one urgent location, false otherwise.
         */
        [[nodiscard]] bool hasUrgentLocations() const;


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


        /**
         * @brief Collects the urgent locations.
         *
         * @param locToIntMap a mapping from locations (represented by std::string) to int.
         * @return a map from integers representing the urgent locations.
         */
        [[nodiscard]] absl::flat_hash_set<int> getUrgentLocations(const std::unordered_map<std::string, int> &locToIntMap) const;


        /**
         * @return a map associating integer variables with their value.
         *
         * @warning The variables are initialized to 0. It is possible to initialize them by adding a transition to the Timed Automaton.
         */
        [[nodiscard]] absl::btree_map<std::string, int> getVariables() const;


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
        std::vector<std::string> integerVariables;
        arena_loc_map locations;
        std::vector<transition> transitions;


        /**
         * @return an int representing the maximum constant appearing in the Timed Arena.
         */
        [[nodiscard]] int getMaxConstant() const;


        /**
         * @brief Computes the maximum constant appearing in a Timed Arena for each clock.
         *
         * @param clocksIndices a map from clock names to their index in the clocks vector.
         * @return a vector containing in position i the maximum constant of the i-th clock, where the index i of the clock is given by clockIndices.
         */
        [[nodiscard]] std::vector<int> getMaxConstants(const std::unordered_map<std::string, int> &clocksIndices) const;


        /**
         * @return true if the Timed Arena has at least one urgent location, false otherwise.
         */
        [[nodiscard]] bool hasUrgentLocations() const;


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


        /**
         * @brief Collects the urgent locations.
         *
         * @param locToIntMap a mapping from locations (represented by std::string) to int.
         * @return a map from integers representing the urgent locations.
         */
        [[nodiscard]] absl::flat_hash_set<int> getUrgentLocations(const std::unordered_map<std::string, int> &locToIntMap) const;


        /**
         * @return a map associating integer variables with their value.
         *
         * @warning The variables are initialized to 0. It is possible to initialize them by adding a transition to the Timed Arena.
         */
        [[nodiscard]] absl::btree_map<std::string, int> getVariables() const;


        [[nodiscard]] std::string to_string() const;
    };
}


inline std::ostream &operator<<(std::ostream &os, const expr::ast::arithmeticExpr &a)
{
    return os << a.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const expr::ast::binaryExpr &b)
{
    return os << b.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const expr::ast::assignmentExpr &a)
{
    return os << a.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const expr::ast::comparisonExpr &c)
{
    return os << c.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const expr::ast::booleanExpr &b)
{
    return os << b.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const expr::ast::booleanBinaryExpr &b)
{
    return os << b.to_string();
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
