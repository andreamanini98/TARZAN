#ifndef AST_H
#define AST_H

#include <boost/optional.hpp>

#include <iostream>
#include <vector>
#include <map>

// TODO: try to insert to_string functions here to print these structs.

namespace ast {
    struct guard {
        std::string clock;
        std::string guardOperator;
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
    using loc_pair = std::pair<std::string, boost::optional<bool>>;
    using loc_map = std::map<std::string, boost::optional<bool>>;

    struct timedAutomaton {
        std::string name;
        std::vector<std::string> clocks;
        std::vector<std::string> actions;
        loc_map locations;
        std::vector<transition> transitions;
    };

    // Defining the map to hold Timed Arenas locations.
    using arena_loc = std::pair<char, boost::optional<bool>>;
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
