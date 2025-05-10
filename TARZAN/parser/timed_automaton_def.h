#ifndef TIMED_AUTOMATON_DEF_H
#define TIMED_AUTOMATON_DEF_H

#include <boost/spirit/home/x3.hpp>

#include "TARZAN/parser/ast.h"
#include "TARZAN/parser/ast_adapted.h"
#include "TARZAN/parser/timed_automaton.h"

namespace timed_automaton {
    namespace parser {
        namespace x3 = boost::spirit::x3;
        namespace ascii = x3::ascii;

        using x3::int_;
        using x3::lit;
        using x3::double_;
        using x3::lexeme;
        using ascii::char_;
        using x3::bool_;

        constexpr x3::rule<guard_class, ast::guard> guard_rule = "guard_rule";
        constexpr x3::rule<transition_class, ast::transition> transition_rule = "transition_rule";
        constexpr x3::rule<timedAutomaton_class, ast::timedAutomaton> timedAutomaton_rule = "timedAutomaton_rule";

        //TODO: enhance the grammar, for example enclose the parts of a timed automaton in braces. something like
        //TODO: can we make already similar to json?
        /*
         * create automaton automaton_name
         * {
         *     clocks
         *     {
         *         x, y, z;
         *     }
         *
         *     actions
         *     {
         *         a, b, c;
         *     }
         *
         *     locations
         *     {
         *         q0, q1, q2, q3;
         *     }
         *
         *     transitions
         *     {
         *     <start, action, [(x, op, 4), (y, op, 5)], [x, y, z], target>,
         *     <start2, action2, [(x2, op2, 4), (y2, op2, 5)], [x2, y2, z2], target2>;
         *     }
         * }
         */

        // you may need to back parentheses in the guard definition, try to mimic the above grammar, and then
        //TODO: extend it to account for arenas.

        constexpr auto parser_string = lexeme[+x3::alnum];

        constexpr auto guard_rule_def =
                parser_string >> lit(',')
                >> parser_string >> lit(',')
                >> int_;

        constexpr auto transition_rule_def =
                lit("transition")
                >> lit('(')
                >> parser_string >> lit(',')
                >> parser_string >> lit(',')
                >> lit('(')
                >> (guard_rule % ',')
                >> lit(')') >> lit(',')
                >> lit('(')
                >> (parser_string % ',')
                >> lit(')') >> lit(',')
                >> parser_string
                >> ')';

        constexpr auto timedAutomaton_rule_def =
                lit("create")
                >> lit("automaton")
                >> lit('{')
                >> (parser_string % ',') >> lit(';')
                >> (parser_string % ',') >> lit(';')
                >> (parser_string % ',') >> lit(';')
                >> (transition_rule % ',') >> lit(';')
                >> lit('}');

        BOOST_SPIRIT_DEFINE(guard_rule, transition_rule, timedAutomaton_rule);
    }


    parser::guard_type guard()
    {
        return parser::guard_rule;
    }


    parser::transition_type transition()
    {
        return parser::transition_rule;
    }


    parser::timedAutomaton_type timedAutomaton()
    {
        return parser::timedAutomaton_rule;
    }
}

#endif //TIMED_AUTOMATON_DEF_H
