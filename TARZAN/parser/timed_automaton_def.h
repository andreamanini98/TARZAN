#ifndef TIMED_AUTOMATON_DEF_H
#define TIMED_AUTOMATON_DEF_H

#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/home/x3.hpp>

#include "TARZAN/parser/ast.h"
#include "TARZAN/parser/ast_adapted.h"
#include "TARZAN/parser/error_handler.h"
#include "TARZAN/parser/timed_automaton.h"
#include "TARZAN/parser/success_handler.h"

namespace timed_automaton {
    inline struct comp_op : boost::spirit::x3::symbols<comparison_op> {
        comp_op()
        {
            auto &self = add
                    ("<", LT)
                    ("<=", LE)
                    ("=", EQ)
                    (">=", GE)
                    (">", GT);
            (void) self;
        }
    } comp_op;

    namespace parser {
        namespace x3 = boost::spirit::x3;
        namespace ascii = x3::ascii;

        using x3::int_;
        using x3::lit;
        using x3::double_;
        using x3::lexeme;
        using ascii::char_;
        using x3::bool_;

        constexpr x3::rule<loc_pair_class, ast::loc_pair> loc_pair_rule = "loc_pair_rule";
        constexpr x3::rule<loc_map_class, ast::loc_map> loc_map_rule = "loc_map_rule";

        constexpr x3::rule<arena_loc_class, ast::arena_loc> arena_loc_rule = "arena_loc_rule";
        constexpr x3::rule<arena_loc_pair_class, ast::arena_loc_pair> arena_loc_pair_rule = "arena_loc_pair_rule";
        constexpr x3::rule<arena_loc_map_class, ast::arena_loc_map> arena_loc_map_rule = "arena_loc_map_rule";

        constexpr x3::rule<guard_class, ast::guard> guard_rule = "guard_rule";
        constexpr x3::rule<transition_class, ast::transition> transition_rule = "transition_rule";
        constexpr x3::rule<timedAutomaton_class, ast::timedAutomaton> timedAutomaton_rule = "timedAutomaton_rule";
        constexpr x3::rule<timedArena_class, ast::timedArena> timedArena_rule = "timedArena_rule";

        inline auto literal =
                lexeme[+char_("a-zA-Z0-9_")];

        inline auto loc_pair_rule_def =
                literal
                > -(lit('<') > lit("initial") > lit(':') > bool_ > lit('>'));

        inline auto loc_map_rule_def =
                loc_pair_rule % ',';

        inline auto arena_loc_rule_def =
                lit('<')
                > lit("player") > lit(':') > (x3::char_('c') | x3::char_('e'))
                > -(lit(',') > lit("initial") > lit(':') > bool_)
                > lit('>');

        inline auto arena_loc_pair_rule_def =
                literal
                > arena_loc_rule;

        inline auto arena_loc_map_rule_def =
                arena_loc_pair_rule % ',';

        inline auto guard_rule_def =
                lit('(')
                > literal > lit(',')
                > comp_op > lit(',')
                > int_
                > lit(')');

        inline auto transition_rule_def =
                lit('(')
                > literal > lit(',')
                > literal > lit(',')
                > lit('[')
                > guard_rule % ','
                > lit(']') > lit(',')
                > lit('[')
                > literal % ','
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

        BOOST_SPIRIT_DEFINE(loc_pair_rule,
                            loc_map_rule,
                            arena_loc_rule,
                            arena_loc_pair_rule,
                            arena_loc_map_rule,
                            guard_rule,
                            transition_rule,
                            timedAutomaton_rule,
                            timedArena_rule);

        struct loc_pair_class : success_handler {
        };

        struct loc_map_class : success_handler {
        };

        struct arena_loc_class : success_handler {
        };

        struct arena_loc_pair_class : success_handler {
        };

        struct arena_loc_map_class : success_handler {
        };

        struct guard_class : success_handler {
        };

        struct transition_class : success_handler {
        };

        struct timedAutomaton_class : error_handler_base, success_handler {
        };

        struct timedArena_class : error_handler_base, success_handler {
        };
    }
}

namespace timed_automaton {
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


    parser::timedArena_type timedArena()
    {
        return parser::timedArena_rule;
    }
}

#endif //TIMED_AUTOMATON_DEF_H
