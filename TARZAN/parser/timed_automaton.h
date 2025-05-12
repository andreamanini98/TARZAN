#ifndef TIMED_AUTOMATON_H
#define TIMED_AUTOMATON_H

#include "TARZAN/parser/ast.h"

#include <boost/spirit/home/x3.hpp>

namespace timed_automaton {
    namespace parser {
        namespace x3 = boost::spirit::x3;

        // Tags for the rules.
        struct loc_pair_class;
        struct loc_map_class;

        struct guard_class;
        struct transition_class;
        struct timedAutomaton_class;

        using guard_type = x3::rule<guard_class, ast::guard>;
        using transition_type = x3::rule<transition_class, ast::transition>;
        using timedAutomaton_type = x3::rule<timedAutomaton_class, ast::timedAutomaton>;

        BOOST_SPIRIT_DECLARE(guard_type, transition_type, timedAutomaton_type);
    }

    parser::guard_type guard();

    parser::transition_type transition();

    parser::timedAutomaton_type timedAutomaton();
}

#endif //TIMED_AUTOMATON_H
