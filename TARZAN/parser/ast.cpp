#include "TARZAN/parser/ast.h"

#include <iostream>
#include "TARZAN/utilities/printing_utilities.h"

using transition = timed_automaton::ast::transition;


// Arithmetic expr class.

int expr::ast::arithmeticExpr::evaluate(const absl::btree_map<std::string, int> &variables) const
{
    return std::visit([&variables]<typename T0>(T0 const &val) -> int {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, int>)
            // Base case: integer literal.
            return val;
        else if constexpr (std::is_same_v<T, variable>)
        {
            // Base case: variable lookup.
            auto it = variables.find(val.name);
            if (it == variables.end())
                throw std::runtime_error("Variable '" + val.name + "' not found!");
            return it->second;
        } else
        {
            // Recursive case: binary expression.
            const binaryExpr &binExpr = val.get();

            const int leftVal = binExpr.left_expr.evaluate(variables);
            const int rightVal = binExpr.right_expr.evaluate(variables);

            switch (binExpr.op)
            {
                case ADD:
                    return leftVal + rightVal;
                case SUB:
                    return leftVal - rightVal;
                case MUL:
                    return leftVal * rightVal;
                case DIV:
                    if (rightVal == 0)
                        throw std::runtime_error("Division by zero!");
                    return leftVal / rightVal;
                default:
                    throw std::runtime_error("Unknown arithmetic operator!");
            }
        }
    }, value);
}


std::string expr::ast::arithmeticExpr::to_string() const
{
    std::ostringstream oss;

    oss << std::visit([]<typename T0>(T0 const &val) -> std::string {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, int>)
            // We must print an integer.
            return std::to_string(val);
        else if constexpr (std::is_same_v<T, variable>)
            // We must print a string (the variable name).
            return val.name;
        else
            // We must print a binary expression.
            return "(" + val.get().to_string() + ")";
    }, value);

    return oss.str();
}


// ---


// Binary expr class.

std::string expr::ast::binaryExpr::to_string() const
{
    std::ostringstream oss;
    oss << left_expr.to_string() << " " << op << " " << right_expr.to_string();
    return oss.str();
}


// ---


// Assignment expr class.

void expr::ast::assignmentExpr::evaluate(absl::btree_map<std::string, int> &variables) const
{
    // Evaluate the right-hand side.
    const int result = rhs.evaluate(variables);

    // Assign the result to the left-hand side variable.
    variables[lhs.name] = result;
}


std::string expr::ast::assignmentExpr::to_string() const
{
    std::ostringstream oss;
    oss << lhs.name << " = " << rhs.to_string();
    return oss.str();
}


// ---


// Comparison expr class.

bool expr::ast::comparisonExpr::evaluate(const absl::btree_map<std::string, int> &variables) const
{
    const int leftVal = left_expr.evaluate(variables);
    const int rightVal = right_expr.evaluate(variables);

    switch (op)
    {
        case LT:
            return leftVal < rightVal;
        case LE:
            return leftVal <= rightVal;
        case EQ:
            return leftVal == rightVal;
        case GE:
            return leftVal >= rightVal;
        case GT:
            return leftVal > rightVal;
        default:
            throw std::runtime_error("Unknown comparison operator!");
    }
}


std::string expr::ast::comparisonExpr::to_string() const
{
    std::ostringstream oss;
    oss << left_expr.to_string() << " " << op << " " << right_expr.to_string();
    return oss.str();
}


// ---


// Boolean expr class.

bool expr::ast::booleanExpr::evaluate(const absl::btree_map<std::string, int> &variables) const
{
    return std::visit([&variables]<typename T0>(T0 const &val) -> bool {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, bool>)
            // Base case: boolean literal (true/false).
            return val;
        else if constexpr (std::is_same_v<T, comparisonExpr>)
        // Base case: comparison expression.
            return val.evaluate(variables);
        else
        {
            // Recursive case: boolean binary expression.
            const booleanBinaryExpr &binExpr = val.get();

            const bool leftVal = binExpr.left_expr.evaluate(variables);

            if (binExpr.op == AND && !leftVal)
                return false;
            if (binExpr.op == OR && leftVal)
                return true;

            const bool rightVal = binExpr.right_expr.evaluate(variables);

            switch (binExpr.op)
            {
                case AND:
                    return leftVal && rightVal;
                case OR:
                    return leftVal || rightVal;
                default:
                    throw std::runtime_error("Unknown boolean operator!");
            }
        }
    }, value);
}


std::string expr::ast::booleanExpr::to_string() const
{
    std::ostringstream oss;

    oss << std::visit([]<typename T0>(T0 const &val) -> std::string {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, bool>)
            return val ? "true" : "false";
        else if constexpr (std::is_same_v<T, comparisonExpr>)
            return val.to_string();
        else
            return "(" + val.get().to_string() + ")";
    }, value);

    return oss.str();
}


// ---


// Boolean binary expr class.

std::string expr::ast::booleanBinaryExpr::to_string() const
{
    std::ostringstream oss;
    const std::string opStr = op == AND ? "AND" : "OR";
    oss << left_expr.to_string() << " " << opStr << " " << right_expr.to_string();
    return oss.str();
}


// ---


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


// LocationContent class.

std::string timed_automaton::ast::locationContent::to_string() const
{
    std::ostringstream oss;
    oss << "<";
    oss << "initial: " << (isInitial ? "true, " : "false, ");
    oss << "urgent: " << (isUrgent ? "true, " : "false, ");
    oss << "invariant: [" << join_elements(invariant, " and ") << "]";
    oss << ">";
    return oss.str();
}


// ---


// Transition class.

bool timed_automaton::ast::transition::isTransitionSatisfied(const std::vector<std::pair<int, bool>> &clockValuation,
                                                             const std::unordered_map<std::string, int> &clocksIndices,
                                                             const absl::btree_map<std::string, int> &variables) const
{
    bool isSatisfied = true;

    // all_of returns true if no elements are in the range.
    if (!clocksIndices.empty())
    {
        isSatisfied = std::ranges::all_of(clockGuard, [&](const auto &cc) {
            const int clockIdx = clocksIndices.at(cc.getClockName());

            const int clockIntVal = clockValuation[clockIdx].first;
            const int clockHasFracPart = clockValuation[clockIdx].second;

            return cc.isSatisfied(clockIntVal, clockHasFracPart);
        });
    }

    if (integerGuard.has_value() && isSatisfied)
        isSatisfied = integerGuard.value().evaluate(variables);

    return isSatisfied;
}


std::string timed_automaton::ast::transition::to_string() const
{
    std::ostringstream oss;
    const std::string actString = action.first + (action.second.has_value() ? in_out_act_to_string(action.second.value()) : "");
    oss << "(" << startingLocation << ", " << actString << ", "
            << "[" << join_elements(clockGuard, " and ") << "], "
            << (integerGuard.has_value() ? integerGuard.value().to_string() + ", " : "")
            << "[" << join_elements(clocksToReset, ", ") << "], "
            << (!integerAssignments.empty() ? "[" + join_elements(integerAssignments, ", ") + "], " : "")
            << targetLocation << ")";
    return oss.str();
}


// ---


// Timed automaton class.

int timed_automaton::ast::timedAutomaton::getMaxConstant() const
{
    int maxConstant{};

    // Checking for maximum constant in transitions.
    for (const transition &tr: transitions)
        for (const clockConstraint &cc: tr.clockGuard)
            if (maxConstant < cc.comparingConstant)
                maxConstant = cc.comparingConstant;

    // Checking for maximum constant in invariants.
    for (const auto &loc: locations | std::views::values)
        for (const clockConstraint &cc: loc.invariant)
            if (maxConstant < cc.comparingConstant)
                maxConstant = cc.comparingConstant;

    return maxConstant;
}


std::vector<int> timed_automaton::ast::timedAutomaton::getMaxConstants(const std::unordered_map<std::string, int> &clocksIndices) const
{
    std::vector<int> maxConstants{};

    if (!clocksIndices.empty())
    {
        maxConstants.resize(clocksIndices.size());

        // Checking for maximum constants in transitions.
        for (const transition &tr: transitions)
            for (const clockConstraint &cc: tr.clockGuard)
                if (maxConstants[clocksIndices.at(cc.clock)] < cc.comparingConstant)
                    maxConstants[clocksIndices.at(cc.clock)] = cc.comparingConstant;

        // Checking for maximum constants in invariants.
        for (const auto &loc: locations | std::views::values)
            for (const clockConstraint &cc: loc.invariant)
                if (maxConstants[clocksIndices.at(cc.clock)] < cc.comparingConstant)
                    maxConstants[clocksIndices.at(cc.clock)] = cc.comparingConstant;
    }
    return maxConstants;
}


bool timed_automaton::ast::timedAutomaton::hasUrgentLocations() const
{
    return std::ranges::any_of(locations | std::views::values, &locationContent::isUrgent);
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


std::vector<int> timed_automaton::ast::timedAutomaton::getInitialLocations(const std::unordered_map<std::string, int> &locToIntMap) const
{
    std::vector<int> initialLocations{};

    for (const auto &[locName, locContent]: locations)
        if (locContent.isInitial)
            initialLocations.push_back(locToIntMap.at(locName));

    return initialLocations;
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


std::vector<std::vector<transition>> timed_automaton::ast::timedAutomaton::getInTransitions(const std::unordered_map<std::string, int> &locToIntMap) const
{
    std::vector<std::vector<transition>> inTransitions;
    inTransitions.resize(locToIntMap.size());

    for (const auto &t: transitions)
    {
        const int idx = locToIntMap.at(t.targetLocation);
        inTransitions[idx].push_back(t);
    }

    return inTransitions;
}


absl::flat_hash_map<int, std::vector<timed_automaton::ast::clockConstraint>> timed_automaton::ast::timedAutomaton::getInvariants(
    const std::unordered_map<std::string, int> &locToIntMap) const
{
    absl::flat_hash_map<int, std::vector<clockConstraint>> invariants;

    for (const auto &[locName, locContent]: locations)
        if (!locContent.invariant.empty())
            invariants[locToIntMap.at(locName)] = locContent.invariant;

    return invariants;
}


absl::flat_hash_set<int> timed_automaton::ast::timedAutomaton::getUrgentLocations(const std::unordered_map<std::string, int> &locToIntMap) const
{
    absl::flat_hash_set<int> urgentLocations;

    for (const auto &[locName, locContent]: locations)
        if (locContent.isUrgent)
            urgentLocations.insert(locToIntMap.at(locName));

    return urgentLocations;
}


absl::btree_map<std::string, int> timed_automaton::ast::timedAutomaton::getVariables() const
{
    absl::btree_map<std::string, int> res{};

    for (const auto &variable: integerVariables)
        res[variable] = 0;

    return res;
}


std::string timed_automaton::ast::timedAutomaton::to_string() const
{
    std::ostringstream oss;
    oss << "Timed Automaton " << name << std::endl;
    oss << "Clocks:\n" << join_elements(clocks, ", ") << std::endl;
    oss << "Actions:\n" << join_elements(actions, ", ") << std::endl;
    oss << "Integer variables:\n" << join_elements(integerVariables, ", ") << std::endl;
    oss << "Locations:\n";
    for (const auto &[key, value]: locations)
    {
        oss << key << ", ";
        oss << value;
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

    // Checking for maximum constant in transitions.
    for (const transition &tr: transitions)
        for (const clockConstraint &cc: tr.clockGuard)
            if (maxConstant < cc.comparingConstant)
                maxConstant = cc.comparingConstant;

    // Checking for maximum constant in invariants.
    for (const auto &[isInitial, locContent]: locations | std::views::values)
        for (const clockConstraint &cc: locContent.invariant)
            if (maxConstant < cc.comparingConstant)
                maxConstant = cc.comparingConstant;

    return maxConstant;
}


std::vector<int> timed_automaton::ast::timedArena::getMaxConstants(const std::unordered_map<std::string, int> &clocksIndices) const
{
    std::vector<int> maxConstants{};

    if (!clocksIndices.empty())
    {
        maxConstants.resize(clocksIndices.size());

        // Checking for maximum constants in transitions.
        for (const transition &tr: transitions)
            for (const clockConstraint &cc: tr.clockGuard)
                if (maxConstants[clocksIndices.at(cc.clock)] < cc.comparingConstant)
                    maxConstants[clocksIndices.at(cc.clock)] = cc.comparingConstant;

        // Checking for maximum constants in invariants.
        for (const auto &[fst, snd]: locations | std::views::values)
            for (const clockConstraint &cc: snd.invariant)
                if (maxConstants[clocksIndices.at(cc.clock)] < cc.comparingConstant)
                    maxConstants[clocksIndices.at(cc.clock)] = cc.comparingConstant;
    }

    return maxConstants;
}


bool timed_automaton::ast::timedArena::hasUrgentLocations() const
{
    return std::ranges::any_of(locations | std::views::values, [](const auto &pair) { return pair.second.isUrgent; });
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


std::vector<int> timed_automaton::ast::timedArena::getInitialLocations(const std::unordered_map<std::string, int> &locToIntMap) const
{
    std::vector<int> initialLocations{};

    for (const auto &[locName, locContent]: locations)
        if (locContent.second.isInitial)
            initialLocations.push_back(locToIntMap.at(locName));

    return initialLocations;
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


std::vector<std::vector<transition>> timed_automaton::ast::timedArena::getInTransitions(const std::unordered_map<std::string, int> &locToIntMap) const
{
    std::vector<std::vector<transition>> inTransitions;
    inTransitions.resize(locToIntMap.size());

    for (const auto &t: transitions)
    {
        const int idx = locToIntMap.at(t.targetLocation);
        inTransitions[idx].push_back(t);
    }

    return inTransitions;
}


absl::flat_hash_map<int, std::vector<timed_automaton::ast::clockConstraint>> timed_automaton::ast::timedArena::getInvariants(
    const std::unordered_map<std::string, int> &locToIntMap) const
{
    absl::flat_hash_map<int, std::vector<clockConstraint>> invariants;

    for (const auto &[locName, locContent]: locations)
        if (!locContent.second.invariant.empty())
            invariants[locToIntMap.at(locName)] = locContent.second.invariant;

    return invariants;
}


absl::flat_hash_set<int> timed_automaton::ast::timedArena::getUrgentLocations(const std::unordered_map<std::string, int> &locToIntMap) const
{
    absl::flat_hash_set<int> urgentLocations;

    for (const auto &[locName, locContent]: locations)
        if (locContent.second.isUrgent)
            urgentLocations.insert(locToIntMap.at(locName));

    return urgentLocations;
}


absl::btree_map<std::string, int> timed_automaton::ast::timedArena::getVariables() const
{
    absl::btree_map<std::string, int> res{};

    for (const auto &variable: integerVariables)
        res[variable] = 0;

    return res;
}


absl::flat_hash_map<int, char> timed_automaton::ast::timedArena::mapLocationsToPlayers(const std::unordered_map<std::string, int> &locToIntMap) const
{
    absl::flat_hash_map<int, char> res{};

    for (const auto &[locName, locContent]: locations)
        res[locToIntMap.at(locName)] = locContent.first;

    return res;
}


std::string timed_automaton::ast::timedArena::to_string() const
{
    std::ostringstream oss;
    oss << "Timed Arena " << name << std::endl;
    oss << "Clocks:\n" << join_elements(clocks, ", ") << std::endl;
    oss << "Actions:\n" << join_elements(actions, ", ") << std::endl;
    oss << "Integer variables:\n" << (integerVariables.empty() ? "[]" : join_elements(integerVariables, ", ")) << std::endl;
    oss << "Locations:\n";
    for (const auto &[location_name, location_info]: locations)
    {
        const auto &[player, locContent] = location_info;
        oss << location_name << ", <" << player << ", ";
        oss << locContent << std::endl;
    }
    oss << "Transitions:\n" << join_elements(transitions, "\n") << std::endl;
    return oss.str();
}
