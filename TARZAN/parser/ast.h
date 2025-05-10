#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>

//TODO: adapt this to arenas (and if necessary to other timed automata specific elements).

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

    struct timedAutomaton {
        std::vector<std::string> clocks;
        std::vector<std::string> actions;
        std::vector<std::string> locations;
        std::vector<transition> transitions;
    };
}

#endif //AST_H
