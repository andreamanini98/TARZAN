#ifndef TARZAN_LIBRARY_H
#define TARZAN_LIBRARY_H

#include <string>

// Core data structures and AST
#include "TARZAN/parser/ast.h"

// Region operations
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"

// Network of Timed Automata
#include "TARZAN/regions/networkOfTA/NetworkRegion.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"

// Enums
#include "TARZAN/regions/enums/state_space_exploration_enum.h"
#include "TARZAN/parser/enums/arithmetic_enum.h"
#include "TARZAN/parser/enums/boolean_op_enum.h"
#include "TARZAN/parser/enums/comparison_op_enum.h"
#include "TARZAN/parser/enums/input_output_action_enum.h"


/**
 * @brief Parses a Timed Automaton from a .txt file.
 *
 * @param path the path in which the file to parse is located.
 * @return a timed_automaton::ast::timedAutomaton representation of the Timed Automaton.
 */
timed_automaton::ast::timedAutomaton parseTimedAutomaton(std::string const &path);


/**
 * @brief Parses all Timed Automata saved as .txt files from a folder.
 *
 * @param folderPath the path in which files to parse are located.
 * @return a std::vector containing the parsed Timed Automata.
 */
std::vector<timed_automaton::ast::timedAutomaton> parseTimedAutomataFromFolder(std::string const &folderPath);


/**
 * @brief Parses a Timed Arena from a .txt file.
 *
 * @param path the path in which the file to parse is located.
 * @return a timed_automaton::ast::timedArena representation of the Timed Arena.
 */
timed_automaton::ast::timedArena parseTimedArena(std::string const &path);


/**
 * @brief Parses all Timed Arenas saved as .txt files from a folder.
 *
 * @param folderPath the path in which files to parse are located.
 * @return a std::vector containing the parsed Timed Arenas.
 */
std::vector<timed_automaton::ast::timedArena> parseTimedArenasFromFolder(std::string const &folderPath);

#endif //TARZAN_LIBRARY_H
