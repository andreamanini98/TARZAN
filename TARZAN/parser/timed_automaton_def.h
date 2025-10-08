#ifndef TIMED_AUTOMATON_DEF_H
#define TIMED_AUTOMATON_DEF_H

#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/home/x3.hpp>

#include "TARZAN/parser/ast.h"
#include "TARZAN/parser/ast_adapted.h"
#include "TARZAN/parser/error_handler.h"
#include "TARZAN/parser/timed_automaton.h"
#include "TARZAN/parser/success_handler.h"


namespace timed_automaton
{
    inline struct comp_op : boost::spirit::x3::symbols<comparison_op>
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

    inline struct io_act : boost::spirit::x3::symbols<in_out_act>
    {
        io_act()
        {
            auto &self = add
                    ("!", OUTACT)
                    ("?", INACT);
            (void) self;
        }
    } io_act;

    inline struct is_ini_loc : boost::spirit::x3::symbols<bool>
    {
        is_ini_loc()
        {
            auto &self = add
                    ("T", true)
                    ("F", false)
                    ("true", true)
                    ("false", false);
            (void) self;
        }
    } is_ini_loc;


    namespace parser
    {
        namespace x3 = boost::spirit::x3;
        namespace ascii = x3::ascii;

        using x3::int_;
        using x3::lit;
        using x3::double_;
        using x3::lexeme;
        using ascii::char_;
        using x3::bool_;

        constexpr x3::rule<action_pair_class, ast::act> action_pair_rule = "action_pair_rule";

        constexpr x3::rule<loc_pair_class, ast::loc_pair> loc_pair_rule = "loc_pair_rule";
        constexpr x3::rule<loc_map_class, ast::loc_map> loc_map_rule = "loc_map_rule";

        constexpr x3::rule<arena_loc_class, ast::arena_loc> arena_loc_rule = "arena_loc_rule";
        constexpr x3::rule<arena_loc_pair_class, ast::arena_loc_pair> arena_loc_pair_rule = "arena_loc_pair_rule";
        constexpr x3::rule<arena_loc_map_class, ast::arena_loc_map> arena_loc_map_rule = "arena_loc_map_rule";

        constexpr x3::rule<clockConstraint_class, ast::clockConstraint> clockConstraint_rule = " clockConstraint_rule";
        constexpr x3::rule<locationContent_class, ast::locationContent> locationContent_rule = "locationContent_rule";
        constexpr x3::rule<transition_class, ast::transition> transition_rule = "transition_rule";
        constexpr x3::rule<timedAutomaton_class, ast::timedAutomaton> timedAutomaton_rule = "timedAutomaton_rule";
        constexpr x3::rule<timedArena_class, ast::timedArena> timedArena_rule = "timedArena_rule";

        inline auto literal =
                lexeme[+char_("a-zA-Z0-9_")];

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
                > (lit("ini") > lit(':') > is_ini_loc
                   > -(lit(',') > lit("inv") > lit(':') > lit('[') > clockConstraint_rule % ',' > lit(']'))
                   |
                   x3::attr(false)
                   > -(lit("inv") > lit(':') > lit('[') > clockConstraint_rule % ',' > lit(']')))
                > lit('>');

        inline auto transition_rule_def =
                lit('(')
                > literal > lit(',')
                > action_pair_rule > lit(',')
                > lit('[')
                > clockConstraint_rule % ','
                > lit(']') > lit(',')
                > lit('[')
                > -(literal % ',') // Since a transition may have no clocks to reset, we put the unused - symbol to match no reset clocks at all.
                > lit(']') > lit(',')
                > literal
                > lit(')');

        inline auto timedAutomaton_rule_def =
                lit("create")
                > lit("automaton")
                > literal
                > lit('{')
                > lit("clocks")
                > lit('{')
                > literal % ',' > lit(';')
                > lit('}')
                > lit("actions")
                > lit('{')
                > literal % ',' > lit(';')
                > lit('}')
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
                > lit('{')
                > lit("clocks")
                > lit('{')
                > literal % ',' > lit(';')
                > lit('}')
                > lit("actions")
                > lit('{')
                > literal % ',' > lit(';')
                > lit('}')
                > lit("locations")
                > lit('{')
                > arena_loc_map_rule > lit(';')
                > lit('}')
                > lit("transitions")
                > lit('{')
                > transition_rule % ',' > lit(';')
                > lit('}')
                > lit('}');

        BOOST_SPIRIT_DEFINE(action_pair_rule,
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
}

namespace timed_automaton
{
    parser::clockConstraint_type clockConstraint()
    {
        return parser::clockConstraint_rule;
    }


    parser::locationContent_type locationContent()
    {
        return parser::locationContent_rule;
    }


    parser::transition_type transition()
    {
        return parser::transition_rule;
    }


    parser::timedAutomaton_type timedAutomaton()
    {
        return parser::timedAutomaton_rule;
    }


    parser::timedArena_type timedArena()
    {
        return parser::timedArena_rule;
    }
}

#endif //TIMED_AUTOMATON_DEF_H
