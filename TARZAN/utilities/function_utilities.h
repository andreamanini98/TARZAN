#ifndef TARZAN_FUNCTION_UTILITIES_H
#define TARZAN_FUNCTION_UTILITIES_H

#include "TARZAN/parser/ast.h"


/// Returns the correct index for accessing a clock in a bitvector.
inline int cIdx(const int numOfClocks, const int i)
{
    return numOfClocks - 1 - i;
}


/**
 * @brief Computes the satisfiability of an invariant.
 *
 * @param invariant a vector of clock constraint whose satisfiability must be computed.
 * @param clockValuation the current clock valuation (integer values and a boolean denoting whether the fractional part is greater than zero).
 * @param clocksIndices the indices of the clocks as they appear in the clocks vector of a Timed Automaton.
 * @return true if the guard is satisfied, false otherwise.
 *
 * @attention Works only if the guard is a conjunction of clock constraints, where a clock constraint is (x ~ c), with ~ in {<, <=, =, >=, >}.
 */
// TODO: si potrebbe creare una struct guardia nell'ast ed usarla solo lì, dovresti anche cambiare le transizioni dopo.
inline bool isInvariantSatisfied(const std::vector<timed_automaton::ast::clockConstraint> &invariant,
                                 const std::vector<std::pair<int, bool>> &clockValuation,
                                 const std::unordered_map<std::string, int> &clocksIndices)
{
    bool isSatisfied = true;

    if (!clocksIndices.empty())
    {
        isSatisfied = std::ranges::all_of(invariant, [&](const auto &cc) {
            const int clockIdx = clocksIndices.at(cc.getClockName());

            const int clockIntVal = clockValuation[clockIdx].first;
            const int clockHasFracPart = clockValuation[clockIdx].second;

            return cc.isSatisfied(clockIntVal, clockHasFracPart);
        });
    }

    return isSatisfied;
}

#endif //TARZAN_FUNCTION_UTILITIES_H
