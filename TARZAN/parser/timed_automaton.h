#ifndef TIMED_AUTOMATON_H
#define TIMED_AUTOMATON_H

#include "TARZAN/parser/ast.h"

#include <boost/spirit/home/x3.hpp>

namespace timed_automaton
{
    namespace parser
    {
        namespace x3 = boost::spirit::x3;

        struct action_pair_class;
        struct loc_pair_class;
        struct loc_map_class;
        struct arena_loc_class;
        struct arena_loc_pair_class;
        struct arena_loc_map_class;
        struct clockConstraint_class;
        struct transition_class;
        struct timedAutomaton_class;
        struct timedArena_class;

        using clockConstraint_type = x3::rule<clockConstraint_class, ast::clockConstraint>;
        using transition_type = x3::rule<transition_class, ast::transition>;
        using timedAutomaton_type = x3::rule<timedAutomaton_class, ast::timedAutomaton>;
        using timedArena_type = x3::rule<timedArena_class, ast::timedArena>;

        BOOST_SPIRIT_DECLARE(clockConstraint_type, transition_type, timedAutomaton_type, timedArena_type);
    }

    parser::clockConstraint_type clockConstraint();

    parser::transition_type transition();

    parser::timedAutomaton_type timedAutomaton();

    parser::timedArena_type timedArena();
}

#endif //TIMED_AUTOMATON_H
