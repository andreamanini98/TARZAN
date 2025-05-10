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

        //TODO: extend it to account for arenas.

        inline auto literal = lexeme[+char_("a-zA-Z0-9_")];

        // TODO: since this depends on the order of the operands, change the comparison_op into a symbol table
        inline auto comp_op = lexeme[x3::string("<=") | x3::string(">=") | x3::string("<") | x3::string(">") | x3::string("=")];


        inline auto guard_rule_def =
                lit('(')
                >> literal >> lit(',')
                >> comp_op >> lit(',') // This should be enhanced to only capture correct operands like <, <=, etc
                >> int_
                >> lit(')');

        inline auto transition_rule_def =
                lit('<')
                >> literal >> lit(',')
                >> literal >> lit(',')
                >> lit('[')
                >> (guard_rule % ',')
                >> lit(']') >> lit(',')
                >> lit('[')
                >> (literal % ',')
                >> lit(']') >> lit(',')
                >> literal
                >> lit('>');

        inline auto timedAutomaton_rule_def =
                lit("create")
                >> lit("automaton")
                >> literal
                >> lit('{')
                >> lit("clocks")
                >> lit('{')
                >> (literal % ',') >> lit(';')
                >> lit('}')
                >> lit("actions")
                >> lit('{')
                >> (literal % ',') >> lit(';')
                >> lit('}')
                >> lit("locations")
                >> lit('{')
                >> (literal % ',') >> lit(';')
                >> lit('}')
                >> lit("transitions")
                >> lit('{')
                >> (transition_rule % ',') >> lit(';')
                >> lit('}')
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
