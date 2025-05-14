#ifndef AST_H
#define AST_H

#include "TARZAN/parser/comparison_op_enum.h"

#include <iostream>
#include <vector>
#include <map>

// TODO: insert to_string functions here (or in another header) to print these structs.

// TODO: Use these to print in the executable and, if possible (but I don't think so) in the on_success handler.

// TODO: comment the code

namespace ast {
    struct guard {
        std::string clock;
        comparison_op guardOperator;
        int comparingConstant;
    };

    struct transition {
        std::string startingLocation;
        std::string action;
        std::vector<guard> clockGuard;
        std::vector<std::string> clocksToReset;
        std::string targetLocation;
    };

    // Defining the map to hold Timed Automata locations.
    using loc_pair = std::pair<std::string, std::optional<bool>>;
    using loc_map = std::map<std::string, std::optional<bool>>;

    struct timedAutomaton {
        std::string name;
        std::vector<std::string> clocks;
        std::vector<std::string> actions;
        loc_map locations;
        std::vector<transition> transitions;
    };

    // Defining the map to hold Timed Arenas locations.
    using arena_loc = std::pair<char, std::optional<bool>>;
    using arena_loc_pair = std::pair<std::string, arena_loc>;
    using arena_loc_map = std::map<std::string, arena_loc>;

    struct timedArena {
        std::string name;
        std::vector<std::string> clocks;
        std::vector<std::string> actions;
        arena_loc_map locations;
        std::vector<transition> transitions;
    };
}

#endif //AST_H
