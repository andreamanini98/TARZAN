#ifndef AST_ADAPTED_H
#define AST_ADAPTED_H

#include "TARZAN/parser/ast.h"

#include <boost/fusion/include/adapt_struct.hpp>


// We do not need to adapt the 'variable' struct, as it consists only of one field (and we construct it with a semantic action).

BOOST_FUSION_ADAPT_STRUCT(expr::ast::binaryExpr, left_expr, op, right_expr)

BOOST_FUSION_ADAPT_STRUCT(expr::ast::assignmentExpr, lhs, rhs)

BOOST_FUSION_ADAPT_STRUCT(timed_automaton::ast::clockConstraint, clock, constraintOperator, comparingConstant)

BOOST_FUSION_ADAPT_STRUCT(timed_automaton::ast::locationContent, isInitial, invariant)

BOOST_FUSION_ADAPT_STRUCT(timed_automaton::ast::transition, startingLocation, action, clockGuard, clocksToReset, targetLocation)

BOOST_FUSION_ADAPT_STRUCT(timed_automaton::ast::timedAutomaton, name, clocks, actions, locations, transitions)

BOOST_FUSION_ADAPT_STRUCT(timed_automaton::ast::timedArena, name, clocks, actions, locations, transitions)

#endif //AST_ADAPTED_H
