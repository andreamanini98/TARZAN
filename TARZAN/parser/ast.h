#ifndef AST_H
#define AST_H

#include "TARZAN/parser/comparison_op_enum.h"
#include "TARZAN/utilities/printing_utilities.h"

#include <vector>
#include <map>
#include <sstream>

// TODO: comment the code

namespace timed_automaton::ast {
    struct clockConstraint {
        std::string clock;
        comparison_op constraintOperator;
        int comparingConstant;


        [[nodiscard]] std::string to_string() const
        {
            std::ostringstream oss;
            oss << clock << " " << constraintOperator << " " << comparingConstant;
            return oss.str();
        }
    };


    struct transition {
        std::string startingLocation;
        std::string action;
        std::vector<clockConstraint> clockGuard;
        std::vector<std::string> clocksToReset;
        std::string targetLocation;


        [[nodiscard]] std::string to_string() const
        {
            std::ostringstream oss;
            oss << "(" << startingLocation << ", " << action << ", "
                    << "[" << join_elements(clockGuard, " and ") << "], "
                    << "[" << join_elements(clocksToReset, ", ") << "], "
                    << targetLocation << ")";
            return oss.str();
        }
    };


    // Defining the map to hold Timed Automata locations.
    using loc_pair = std::pair<std::string, std::optional<bool>>;
    using loc_map = std::unordered_map<std::string, std::optional<bool>>;


    struct timedAutomaton {
        std::string name;
        std::vector<std::string> clocks;
        std::vector<std::string> actions;
        loc_map locations;
        std::vector<transition> transitions;


        [[nodiscard]] std::string to_string() const
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
            oss << "Transitions:\n" << join_elements(transitions, "\n") << std::endl;
            return oss.str();
        }
    };


    // Defining the map to hold Timed Arenas locations.
    using arena_loc = std::pair<char, std::optional<bool>>;
    using arena_loc_pair = std::pair<std::string, arena_loc>;
    using arena_loc_map = std::unordered_map<std::string, arena_loc>;


    struct timedArena {
        std::string name;
        std::vector<std::string> clocks;
        std::vector<std::string> actions;
        arena_loc_map locations;
        std::vector<transition> transitions;


        [[nodiscard]] std::string to_string() const
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


inline std::ostream &operator<<(std::ostream &os, const timed_automaton::ast::clockConstraint &g)
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
