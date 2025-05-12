#ifndef AST_H
#define AST_H

#include <boost/optional.hpp>

#include <iostream>
#include <vector>
#include <map>

//TODO: adapt this to arenas (and if necessary to other timed automata specific elements).

// TODO: try to insert to_string functions here to print these structs.

namespace ast {
    // Defining the map to hold Timed Automata locations.
    using loc_pair = std::pair<std::string, boost::optional<bool>>;
    using loc_map = std::map<std::string, boost::optional<bool>>;

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

    struct timedAutomaton {
        std::string name;
        std::vector<std::string> clocks;
        std::vector<std::string> actions;
        loc_map locations;
        std::vector<transition> transitions;
    };
}

#endif //AST_H
