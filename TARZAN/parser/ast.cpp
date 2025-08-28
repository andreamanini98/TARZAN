#include "TARZAN/parser/ast.h"

#include <iostream>

using transition = timed_automaton::ast::transition;


// Clock constraint class.

bool timed_automaton::ast::clockConstraint::isSatisfied(const int clockValue, const bool isFractionalPartGreaterThanZero) const
{
    switch (constraintOperator)
    {
        case LT:
            // No need of fractional part info.
            return clockValue < comparingConstant;

        case LE:
            if (isFractionalPartGreaterThanZero)
                return clockValue < comparingConstant;
            return clockValue <= comparingConstant;

        case EQ:
            if (isFractionalPartGreaterThanZero)
                return false;
            return clockValue == comparingConstant;

        case GE:
            // No need of fractional part info.
            return clockValue >= comparingConstant;

        case GT:
            if (isFractionalPartGreaterThanZero)
                return clockValue >= comparingConstant;
            return clockValue > comparingConstant;

        default:
            std::cerr << "Unsupported constraintOperator: " << constraintOperator << std::endl;
            return false;
    }
}


std::string timed_automaton::ast::clockConstraint::to_string() const
{
    std::ostringstream oss;
    oss << clock << " " << constraintOperator << " " << comparingConstant;
    return oss.str();
}


// ---


// Transition class.

bool timed_automaton::ast::transition::isGuardSatisfied(const std::vector<std::pair<int, bool>> &clockValuation,
                                                        const std::unordered_map<std::string, int> &clocksIndices) const
{
    return std::ranges::all_of(clockGuard, [&](const auto &cc) {
        const int clockIdx = clocksIndices.at(cc.getClockName());

        const int clockIntVal = clockValuation[clockIdx].first;
        const int clockHasFracPart = clockValuation[clockIdx].second;

        return cc.isSatisfied(clockIntVal, clockHasFracPart);
    });
}


std::string timed_automaton::ast::transition::to_string() const
{
    std::ostringstream oss;
    oss << "(" << startingLocation << ", " << action << ", "
            << "[" << join_elements(clockGuard, " and ") << "], "
            << "[" << join_elements(clocksToReset, ", ") << "], "
            << targetLocation << ")";
    return oss.str();
}


// ---


// Timed automaton class.

int timed_automaton::ast::timedAutomaton::getMaxConstant() const
{
    int maxConstant{};
    for (const transition &tr: transitions)
        for (const clockConstraint &cc: tr.clockGuard)
            if (maxConstant < cc.comparingConstant)
                maxConstant = cc.comparingConstant;
    return maxConstant;
}


std::unordered_map<std::string, int> timed_automaton::ast::timedAutomaton::getClocksIndices() const
{
    std::unordered_map<std::string, int> indices{};

    for (int i = 0; i < static_cast<int>(clocks.size()); i++)
        indices.insert({ clocks[i], i });

    return indices;
}


std::unordered_map<std::string, int> timed_automaton::ast::timedAutomaton::mapLocationsToInt() const
{
    std::unordered_map<std::string, int> map;
    int idx{};

    for (const auto &loc: locations | std::views::keys)
    {
        map[loc] = idx;
        idx++;
    }

    return map;
}


std::vector<std::vector<transition>> timed_automaton::ast::timedAutomaton::getOutTransitions(const std::unordered_map<std::string, int> &locToIntMap) const
{
    std::vector<std::vector<transition>> outTransitions;
    outTransitions.resize(locToIntMap.size());

    for (const auto &t: transitions)
    {
        const int idx = locToIntMap.at(t.startingLocation);
        outTransitions[idx].push_back(t);
    }

    return outTransitions;
}


std::string timed_automaton::ast::timedAutomaton::to_string() const
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


// ---


// Timed arena class

int timed_automaton::ast::timedArena::getMaxConstant() const
{
    int maxConstant{};
    for (const transition &tr: transitions)
        for (const clockConstraint &cc: tr.clockGuard)
            if (maxConstant < cc.comparingConstant)
                maxConstant = cc.comparingConstant;
    return maxConstant;
}


std::unordered_map<std::string, int> timed_automaton::ast::timedArena::getClocksIndices() const
{
    std::unordered_map<std::string, int> indices{};

    for (int i = 0; i < static_cast<int>(clocks.size()); i++)
        indices.insert({ clocks[i], i });

    return indices;
}


std::unordered_map<std::string, int> timed_automaton::ast::timedArena::mapLocationsToInt() const
{
    std::unordered_map<std::string, int> map;
    int idx{};

    for (const auto &loc: locations | std::views::keys)
    {
        map[loc] = idx;
        idx++;
    }

    return map;
}


std::vector<std::vector<transition>> timed_automaton::ast::timedArena::getOutTransitions(const std::unordered_map<std::string, int> &locToIntMap) const
{
    std::vector<std::vector<transition>> outTransitions;
    outTransitions.resize(locToIntMap.size());

    for (const auto &t: transitions)
    {
        const int idx = locToIntMap.at(t.startingLocation);
        outTransitions[idx].push_back(t);
    }

    return outTransitions;
}


std::string timed_automaton::ast::timedArena::to_string() const
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
