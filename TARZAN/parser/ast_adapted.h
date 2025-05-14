#ifndef AST_ADAPTED_H
#define AST_ADAPTED_H

#include "TARZAN/parser/ast.h"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(timed_automaton::ast::guard, clock, guardOperator, comparingConstant)

BOOST_FUSION_ADAPT_STRUCT(timed_automaton::ast::transition, startingLocation, action, clockGuard, clocksToReset, targetLocation)

BOOST_FUSION_ADAPT_STRUCT(timed_automaton::ast::timedAutomaton, name, clocks, actions, locations, transitions)

BOOST_FUSION_ADAPT_STRUCT(timed_automaton::ast::timedArena, name, clocks, actions, locations, transitions)

#endif //AST_ADAPTED_H
