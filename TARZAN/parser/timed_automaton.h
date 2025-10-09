#ifndef TIMED_AUTOMATON_H
#define TIMED_AUTOMATON_H

#include "TARZAN/parser/ast.h"

#include <boost/spirit/home/x3.hpp>


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
    struct locationContent_class;
    struct transition_class;
    struct timedAutomaton_class;
    struct timedArena_class;

    using clockConstraint_type = x3::rule<clockConstraint_class, timed_automaton::ast::clockConstraint>;
    using locationContent_type = x3::rule<locationContent_class, timed_automaton::ast::locationContent>;
    using transition_type = x3::rule<transition_class, timed_automaton::ast::transition>;
    using timedAutomaton_type = x3::rule<timedAutomaton_class, timed_automaton::ast::timedAutomaton>;
    using timedArena_type = x3::rule<timedArena_class, timed_automaton::ast::timedArena>;

    BOOST_SPIRIT_DECLARE(clockConstraint_type, locationContent_type, transition_type, timedAutomaton_type, timedArena_type);


    clockConstraint_type clockConstraint();

    locationContent_type locationContent();

    transition_type transition();

    timedAutomaton_type timedAutomaton();

    timedArena_type timedArena();
}


namespace parser
{
    namespace x3 = boost::spirit::x3;

    struct primary_class;
    struct multiplicative_class;
    struct additive_class;
    struct arithmeticExpr_class;
    struct variable_class;
    struct assignmentExpr_class;

    using variable_type = x3::rule<variable_class, expr::ast::variable>;
    using assignmentExpr_type = x3::rule<assignmentExpr_class, expr::ast::assignmentExpr>;

    BOOST_SPIRIT_DECLARE(variable_type, assignmentExpr_type);


    variable_type variable();

    assignmentExpr_type assignmentExpr();
}

#endif //TIMED_AUTOMATON_H
