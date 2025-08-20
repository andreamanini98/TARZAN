#ifndef TARZAN_LIBRARY_H
#define TARZAN_LIBRARY_H

#include <string>

#include "TARZAN/parser/ast.h"

// TODO: insert the grammar of the parser in the comments here (or somewhere else).


/**
 * @brief Parses a Timed Automaton from a .txt file.
 *
 * @param path the path in which the file to parse is located.
 * @return a timed_automaton::ast::timedAutomaton representation of the Timed Automaton.
 */
timed_automaton::ast::timedAutomaton parseTimedAutomaton(std::string const &path);


/**
 * @brief Parses a Timed Arena from a .txt file.
 *
 * @param path the path in which the file to parse is located.
 * @return a timed_automaton::ast::timedArena representation of the Timed Arena.
 */
timed_automaton::ast::timedArena parseTimedArena(std::string const &path);

#endif //TARZAN_LIBRARY_H
