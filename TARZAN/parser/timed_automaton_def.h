#ifndef TIMED_AUTOMATON_DEF_H
#define TIMED_AUTOMATON_DEF_H

#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/home/x3.hpp>

#include "TARZAN/parser/ast.h"
#include "TARZAN/parser/ast_adapted.h"
#include "TARZAN/parser/error_handler.h"
#include "TARZAN/parser/timed_automaton.h"
#include "TARZAN/parser/success_handler.h"


namespace parser
{
    // Symbol tables.
    inline struct arith_op : x3::symbols<arithmetic_op>
    {
        arith_op()
        {
            auto &self = add
                    ("+", ADD)
                    ("-", SUB)
                    ("*", MUL)
                    ("/", DIV);
            (void) self;
        }
    } arith_op;

    inline struct mult_op : x3::symbols<arithmetic_op>
    {
        mult_op()
        {
            auto &self = add
                    ("*", MUL)
                    ("/", DIV);
            (void) self;
        }
    } mult_op;

    inline struct add_op : x3::symbols<arithmetic_op>
    {
        add_op()
        {
            auto &self = add
                    ("+", ADD)
                    ("-", SUB);
            (void) self;
        }
    } add_op;

    inline struct assign_op : x3::symbols<assignment_op>
    {
        assign_op()
        {
            auto &self = add
                    ("=", ASS);
            (void) self;
        }
    } assign_op;

    inline struct and_op : x3::symbols<boolean_op>
    {
        and_op()
        {
            auto &self = add
                    ("&&", AND);
            (void) self;
        }
    } and_op;

    inline struct or_op : x3::symbols<boolean_op>
    {
        or_op()
        {
            auto &self = add
                    ("||", OR);
            (void) self;
        }
    } or_op;

    inline struct comp_op : x3::symbols<comparison_op>
    {
        comp_op()
        {
            auto &self = add
                    ("<", LT)
                    ("<=", LE)
                    ("==", EQ)
                    (">=", GE)
                    (">", GT);
            (void) self;
        }
    } comp_op;

    inline struct io_act : x3::symbols<in_out_act>
    {
        io_act()
        {
            auto &self = add
                    ("!", OUTACT)
                    ("?", INACT);
            (void) self;
        }
    } io_act;

    inline struct my_boolean : x3::symbols<bool>
    {
        my_boolean()
        {
            auto &self = add
                    ("T", true)
                    ("F", false)
                    ("true", true)
                    ("false", false);
            (void) self;
        }
    } my_boolean;


    namespace x3 = boost::spirit::x3;
    namespace ascii = x3::ascii;

    using x3::int_;
    using x3::lit;
    using x3::double_;
    using x3::lexeme;
    using ascii::char_;
    using x3::bool_;

    constexpr x3::rule<primary_class, expr::ast::arithmeticExpr> primary_rule = "primary_rule";
    constexpr x3::rule<multiplicative_class, expr::ast::arithmeticExpr> multiplicative_rule = "multiplicative_rule";
    constexpr x3::rule<additive_class, expr::ast::arithmeticExpr> additive_rule = "additive_rule";

    constexpr x3::rule<arithmeticExpr_class, expr::ast::arithmeticExpr> arithmeticExpr_rule = "arithmeticExpr_rule";
    constexpr x3::rule<variable_class, expr::ast::variable> variable_rule = "variable_rule";
    constexpr x3::rule<assignmentExpr_class, expr::ast::assignmentExpr> assignmentExpr_rule = "assignmentExpr_rule";

    constexpr x3::rule<booleanAndTerm_class, expr::ast::booleanExpr> booleanAndTerm_rule = "booleanAndTerm_rule";
    constexpr x3::rule<booleanOrTerm_class, expr::ast::booleanExpr> booleanOrTerm_rule = "booleanOrTerm_rule";

    constexpr x3::rule<comparisonExpr_class, expr::ast::comparisonExpr> comparisonExpr_rule = "comparisonExpr_rule";
    constexpr x3::rule<booleanTerm_class, expr::ast::booleanExpr> booleanTerm_rule = "booleanTerm_rule";
    constexpr x3::rule<booleanExpr_class, expr::ast::booleanExpr> booleanExpr_rule = "booleanExpr_rule";

    constexpr x3::rule<action_pair_class, timed_automaton::ast::act> action_pair_rule = "action_pair_rule";

    constexpr x3::rule<loc_pair_class, timed_automaton::ast::loc_pair> loc_pair_rule = "loc_pair_rule";
    constexpr x3::rule<loc_map_class, timed_automaton::ast::loc_map> loc_map_rule = "loc_map_rule";

    constexpr x3::rule<arena_loc_class, timed_automaton::ast::arena_loc> arena_loc_rule = "arena_loc_rule";
    constexpr x3::rule<arena_loc_pair_class, timed_automaton::ast::arena_loc_pair> arena_loc_pair_rule = "arena_loc_pair_rule";
    constexpr x3::rule<arena_loc_map_class, timed_automaton::ast::arena_loc_map> arena_loc_map_rule = "arena_loc_map_rule";

    constexpr x3::rule<clockConstraint_class, timed_automaton::ast::clockConstraint> clockConstraint_rule = " clockConstraint_rule";
    constexpr x3::rule<locationContent_class, timed_automaton::ast::locationContent> locationContent_rule = "locationContent_rule";
    constexpr x3::rule<transition_class, timed_automaton::ast::transition> transition_rule = "transition_rule";
    constexpr x3::rule<timedAutomaton_class, timed_automaton::ast::timedAutomaton> timedAutomaton_rule = "timedAutomaton_rule";
    constexpr x3::rule<timedArena_class, timed_automaton::ast::timedArena> timedArena_rule = "timedArena_rule";


    inline auto literal =
            lexeme[+char_("a-zA-Z0-9_")];

    // --- EXPRESSION RULES --- //

    // We explicitly construct a variable using a semantic action.
    inline auto const variable_rule_def =
            literal[([](auto &ctx) { _val(ctx) = expr::ast::variable{ _attr(ctx) }; })];

    // Primary expressions used to avoid infinite recursion during parsing.
    inline auto primary_rule_def =
            int_
            | variable_rule
            | lit('(') > additive_rule > lit(')');

    // Multiplicative expressions that have higher precedence than additive expressions.
    // We directly construct a binary expression using a semantic action.
    inline auto multiplicative_rule_def =
            primary_rule[([](auto &ctx) { _val(ctx) = _attr(ctx); })]
            > *(mult_op > primary_rule)[([](auto &ctx) {
                auto &val = _val(ctx);
                const auto op = boost::get<arithmetic_op>(at_c<0>(_attr(ctx)));
                const auto &right = at_c<1>(_attr(ctx));
                val = expr::ast::binaryExpr{ std::move(val), op, std::move(right) };
            })];

    // Additive expressions that have lower precedence than multiplicative expressions.
    // We directly construct a binary expression using a semantic action.
    inline auto additive_rule_def =
            multiplicative_rule[([](auto &ctx) { _val(ctx) = _attr(ctx); })]
            > *(add_op > multiplicative_rule)[([](auto &ctx) {
                auto &val = _val(ctx);
                const auto op = boost::get<arithmetic_op>(at_c<0>(_attr(ctx)));
                const auto &right = at_c<1>(_attr(ctx));
                val = expr::ast::binaryExpr{ std::move(val), op, std::move(right) };
            })];

    inline auto arithmeticExpr_rule_def =
            additive_rule;

    inline auto assignmentExpr_rule_def =
            variable_rule
            > lit('=') > arithmeticExpr_rule;

    // The parsing of booleanExpr must happen first to avoid ambiguities with primary_rule.
    inline auto booleanTerm_rule_def =
            lit('(') > booleanOrTerm_rule > lit(')')
            | comparisonExpr_rule
            | my_boolean;

    // Logical AND (&&) has higher precedence than OR (||).
    inline auto booleanAndTerm_rule_def =
            booleanTerm_rule[([](auto &ctx) { _val(ctx) = _attr(ctx); })]
            > *(and_op > booleanTerm_rule)[([](auto &ctx) {
                auto &val = _val(ctx);
                const auto op = boost::get<boolean_op>(at_c<0>(_attr(ctx)));
                const auto &right = at_c<1>(_attr(ctx));
                val = expr::ast::booleanBinaryExpr{ std::move(val), op, std::move(right) };
            })];

    // Logical OR (||) has lower precedence than AND (&&).
    inline auto booleanOrTerm_rule_def =
            booleanAndTerm_rule[([](auto &ctx) { _val(ctx) = _attr(ctx); })]
            > *(or_op > booleanAndTerm_rule)[([](auto &ctx) {
                auto &val = _val(ctx);
                const auto op = boost::get<boolean_op>(at_c<0>(_attr(ctx)));
                const auto &right = at_c<1>(_attr(ctx));
                val = expr::ast::booleanBinaryExpr{ std::move(val), op, std::move(right) };
            })];

    inline auto const comparisonExpr_rule_def =
            arithmeticExpr_rule
            > comp_op
            > arithmeticExpr_rule;

    inline auto booleanExpr_rule_def =
            booleanOrTerm_rule;

    // --- TIMED AUTOMATON AND ARENA RULES --- //

    inline auto action_pair_rule_def =
            literal
            > -io_act;

    inline auto loc_pair_rule_def =
            literal
            > locationContent_rule;

    inline auto loc_map_rule_def =
            loc_pair_rule % ',';

    inline auto arena_loc_rule_def =
            lit('<')
            > lit("player") > lit(':') > (x3::char_('c') | x3::char_('e'))
            > lit(',') > locationContent_rule
            > lit('>');

    inline auto arena_loc_pair_rule_def =
            literal
            > arena_loc_rule;

    inline auto arena_loc_map_rule_def =
            arena_loc_pair_rule % ',';

    inline auto clockConstraint_rule_def =
            lit('(')
            > literal > lit(',')
            > comp_op > lit(',')
            > int_
            > lit(')');

    inline auto locationContent_rule_def =
            lit('<')
            >> (
                // ini, urg, -inv
                lit("ini") >> lit(':') >> my_boolean
                >> lit(',') >> lit("urg") >> lit(':') >> my_boolean
                >> -(lit(',') >> lit("inv") >> lit(':') >> lit('[') >> clockConstraint_rule % ',' >> lit(']'))
                |
                // ini, inv (no urg)
                lit("ini") >> lit(':') >> my_boolean
                >> x3::attr(false)
                >> lit(',') >> lit("inv") >> lit(':') >> lit('[') >> clockConstraint_rule % ',' >> lit(']')
                |
                // ini (no urg, no inv)
                lit("ini") >> lit(':') >> my_boolean
                >> x3::attr(false)
                >> x3::attr(std::vector<timed_automaton::ast::clockConstraint>{})
                |
                // urg, -inv (no ini)
                x3::attr(false)
                >> lit("urg") >> lit(':') >> my_boolean
                >> -(lit(',') >> lit("inv") >> lit(':') >> lit('[') >> clockConstraint_rule % ',' >> lit(']'))
                |
                // inv (no ini, no urg)
                x3::attr(false)
                >> x3::attr(false)
                >> lit("inv") >> lit(':') >> lit('[') >> clockConstraint_rule % ',' >> lit(']')
                |
                // urg (no ini, no inv)
                x3::attr(false)
                >> lit("urg") >> lit(':') >> my_boolean
                >> x3::attr(std::vector<timed_automaton::ast::clockConstraint>{})
            )
            > lit('>');


    inline auto transition_rule_def =
            lit('(')
            > literal > lit(',')
            > action_pair_rule > lit(',')
            > lit('[')
            > -(clockConstraint_rule % ',') // A transition may not have a clock guard at all.
            > lit(']') > lit(',')
            > -(booleanExpr_rule > lit(','))
            > lit('[')
            > -(literal % ',') // Since a transition may have no clocks to reset, we put the unused - symbol to match no reset clocks at all.
            > lit(']') > lit(',')
            > -(lit('[') > assignmentExpr_rule % ',' > lit(']') > lit(','))
            > literal
            > lit(')');

    inline auto symm_rule =
            lit("::") > lit("symm") > lit('<') > int_ > lit('>');


    inline auto timedAutomaton_rule_def =
            lit("create")
            > lit("automaton")
            > literal
            > -symm_rule
            > lit('{')
            > lit("clocks")
            > lit('{')
            > -(literal % ',' > lit(';'))
            > lit('}')
            > lit("actions")
            > lit('{')
            > literal % ',' > lit(';')
            > lit('}')
            > -(lit("integers")
                > lit('{')
                > literal % ',' > lit(';')
                > lit('}'))
            > lit("locations")
            > lit('{')
            > loc_map_rule > lit(';')
            > lit('}')
            > lit("transitions")
            > lit('{')
            > transition_rule % ',' > lit(';')
            > lit('}')
            > lit('}');

    inline auto timedArena_rule_def =
            lit("create")
            > lit("arena")
            > literal
            > -symm_rule
            > lit('{')
            > lit("clocks")
            > lit('{')
            > -(literal % ',' > lit(';'))
            > lit('}')
            > lit("actions")
            > lit('{')
            > literal % ',' > lit(';')
            > lit('}')
            > -(lit("integers")
                > lit('{')
                > literal % ',' > lit(';')
                > lit('}'))
            > lit("locations")
            > lit('{')
            > arena_loc_map_rule > lit(';')
            > lit('}')
            > lit("transitions")
            > lit('{')
            > transition_rule % ',' > lit(';')
            > lit('}')
            > lit('}');

    BOOST_SPIRIT_DEFINE(primary_rule,
                        multiplicative_rule,
                        additive_rule,
                        arithmeticExpr_rule,
                        variable_rule,
                        assignmentExpr_rule,
                        booleanTerm_rule,
                        booleanAndTerm_rule,
                        booleanOrTerm_rule,
                        comparisonExpr_rule,
                        booleanExpr_rule,
                        action_pair_rule,
                        loc_pair_rule,
                        loc_map_rule,
                        arena_loc_rule,
                        arena_loc_pair_rule,
                        arena_loc_map_rule,
                        clockConstraint_rule,
                        locationContent_rule,
                        transition_rule,
                        timedAutomaton_rule,
                        timedArena_rule);

    struct primary_class : success_handler
    {};

    struct multiplicative_class : success_handler
    {};

    struct additive_class : success_handler
    {};

    struct arithmeticExpr_class : success_handler
    {};

    struct variable_class : success_handler
    {};

    struct binary_expr_class : success_handler
    {};

    struct assignmentExpr_class : error_handler_base, success_handler
    {};

    struct booleanTerm_class : success_handler
    {};

    struct booleanAndTerm_class : success_handler
    {};

    struct booleanOrTerm_class : success_handler
    {};

    struct comparisonExpr_class : success_handler
    {};

    struct booleanExpr_class : error_handler_base, success_handler
    {};

    struct action_pair_class : success_handler
    {};

    struct loc_pair_class : success_handler
    {};

    struct loc_map_class : success_handler
    {};

    struct arena_loc_class : success_handler
    {};

    struct arena_loc_pair_class : success_handler
    {};

    struct arena_loc_map_class : success_handler
    {};

    struct clockConstraint_class : success_handler
    {};

    struct locationContent_class : success_handler
    {};

    struct transition_class : success_handler
    {};

    struct timedAutomaton_class : error_handler_base, success_handler
    {};

    struct timedArena_class : error_handler_base, success_handler
    {};
}


namespace timed_automaton
{
    // NOLINTNEXTLINE
    inline parser::clockConstraint_type clockConstraint()
    {
        return parser::clockConstraint_rule;
    }


    // NOLINTNEXTLINE
    inline parser::locationContent_type locationContent()
    {
        return parser::locationContent_rule;
    }


    // NOLINTNEXTLINE
    inline parser::transition_type transition()
    {
        return parser::transition_rule;
    }


    // NOLINTNEXTLINE
    inline parser::timedAutomaton_type timedAutomaton()
    {
        return parser::timedAutomaton_rule;
    }


    // NOLINTNEXTLINE
    inline parser::timedArena_type timedArena()
    {
        return parser::timedArena_rule;
    }
}


namespace expr
{
    // NOLINTNEXTLINE
    inline parser::variable_type variable()
    {
        return parser::variable_rule;
    }


    // NOLINTNEXTLINE
    inline parser::assignmentExpr_type assignmentExpr()
    {
        return parser::assignmentExpr_rule;
    }


    // NOLINTNEXTLINE
    inline parser::booleanExpr_type booleanExpr()
    {
        return parser::booleanExpr_rule;
    }
}

#endif //TIMED_AUTOMATON_DEF_H
