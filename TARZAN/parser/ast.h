#ifndef AST_H
#define AST_H

#include "TARZAN/parser/comparison_op_enum.h"
#include "TARZAN/utilities/printing_utilities.h"

#include <vector>
#include <map>
#include <sstream>

// TODO: comment the code

namespace timed_automaton::ast {
    // TODO: this should be called "clock constraint"
    struct guard {
        std::string clock;
        comparison_op guardOperator; // TODO: also this must be called clock constraint operator
        int comparingConstant;


        std::string to_string() const
        {
            std::ostringstream oss;
            oss << clock << " " << guardOperator << " " << comparingConstant;
            return oss.str();
        }
    };


    struct transition {
        std::string startingLocation;
        std::string action;
        std::vector<guard> clockGuard; // TODO; maybe find a better name, a 'guard' should be called a clock constraint
        std::vector<std::string> clocksToReset;
        std::string targetLocation;


        std::string to_string() const
        {
            std::ostringstream oss;
            oss << "(" << startingLocation << ", " << action << ", " << "[";
            // TODO: rename also here to clock constraint
            oss << join_elements(clockGuard, " and ");
            oss << "], [";
            oss << join_elements(clocksToReset, ", "), // TODO: rename also here to clock constraint
                    oss << "], ";
            oss << targetLocation << ")";
            return oss.str();
        }
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


        std::string to_string() const
        {
            std::ostringstream oss;
            oss << "Timed Automaton " << name << std::endl;
            oss << "Clocks:\n" << join_elements(clocks, ", ") << std::endl;
            oss << "Actions:\n" << join_elements(actions, ", ") << std::endl;
            oss << "Locations:\n";
            for (const auto &[key, value]: locations)
            {
                oss << key << ", ";
                if (value.has_value())
                    oss << (value.value() ? "true" : "false");
                else
                    oss << "null_opt";
                oss << std::endl;
            }
            oss << "Transitions: " << join_elements(transitions, "\n") << std::endl;
            return oss.str();
        }
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


        std::string to_string() const
        {
            std::ostringstream oss;
            oss << "Timed Arena " << name << std::endl;
            oss << "Clocks:\n" << join_elements(clocks, ", ") << std::endl;
            oss << "Actions:\n" << join_elements(actions, ", ") << std::endl;
            oss << "Locations:\n";
            for (const auto &[location_name, location_info]: locations)
            {
                const auto &[player, initial] = location_info;
                oss << location_name << ", <" << player << ", ";
                if (initial.has_value())
                    oss << (initial.value() ? "true" : "false");
                else
                    oss << "null_opt";
                oss << ">\n";
            }
            oss << "Transitions:\n" << join_elements(transitions, "\n") << std::endl;
            return oss.str();
        }
    };
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::guard &g)
{
    return os << g.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::transition &t)
{
    return os << t.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::timedAutomaton &t)
{
    return os << t.to_string();
}


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::timedArena &t)
{
    return os << t.to_string();
}

#endif //AST_H
