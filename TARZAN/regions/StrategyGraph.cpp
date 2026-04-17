#include "StrategyGraph.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <queue>
#include <unordered_set>

#include "RTSArena.h"


void region::StrategyGraph::addStrategyTransition(const Region &source, const transition &arenaTransition, const Region &target, const clockValuation &cv)
{
    strategyTransitions[source].emplace(arenaTransition, target, cv);
}


strategyTransitionSet region::StrategyGraph::getStrategyTransitionsGivenSource(const Region &source) const
{
    if (const auto it = strategyTransitions.find(source); it != strategyTransitions.end())
        return it->second;
    return {};
}


inline void region::StrategyGraph::printClockValuationInStrategy(const std::vector<std::pair<int, bool>> &cv,
                                                                 const std::unordered_map<int, std::string> &indicesToClocks,
                                                                 const std::string &indent)
{
    for (int i = 0; i < static_cast<int>(cv.size()); i++)
        std::cout << indent << indicesToClocks.at(i) << " := (" << cv[i].first << ", " << (cv[i].second ? "frac > 0" : "frac = 0") << ")\n";
}


inline void region::StrategyGraph::printRegionInStrategy(const Region &reg,
                                                         const std::unordered_map<int, std::string> &intToLocations,
                                                         const std::unordered_map<int, std::string> &indicesToClocks,
                                                         const absl::flat_hash_map<int, players_sym> &locationsToPlayers,
                                                         const std::string &indent)
{
    const int regLocation = reg.getLocation();
    std::cout << indent << intToLocations.at(regLocation) << " [" << locationsToPlayers.at(regLocation) << "]\n";
    printClockValuationInStrategy(reg.getClockValuation(), indicesToClocks, indent);
}


void region::StrategyGraph::printRegionWithBox(const int step,
                                               const Region &reg,
                                               const std::unordered_map<int, std::string> &intToLocations,
                                               const std::unordered_map<int, std::string> &indicesToClocks,
                                               const absl::flat_hash_map<int, players_sym> &locationsToPlayers,
                                               const std::string &symbolNextToStep)
{
    std::cout << "  \u250c" << repeatString("\u2500", BOX_WIDTH) << "\u2510\n";
    std::cout << "    Step " << step << symbolNextToStep << "\n";
    std::cout << "    " << repeatString("\u2500", 10) << "\n";
    printRegionInStrategy(reg, intToLocations, indicesToClocks, locationsToPlayers, "    ");
    std::cout << "  \u2514" << repeatString("\u2500", BOX_WIDTH) << "\u2518\n";
}


void region::StrategyGraph::printStrategyTransition(const int step,
                                                    const Region &current,
                                                    const std::unordered_map<int, std::string> &intToLocations,
                                                    const std::unordered_map<int, std::string> &indicesToClocks,
                                                    const absl::flat_hash_map<int, players_sym> &locationsToPlayers,
                                                    const clockValuation &moveClockValuation,
                                                    const transition &arenaTransition)
{
    // Print the current region.
    printRegionWithBox(step, current, intToLocations, indicesToClocks, locationsToPlayers, "");

    // Print the move details.
    std::cout << "          \u2502\n";
    std::cout << "          \u251c\u2500 Move clock valuation:\n";
    printClockValuationInStrategy(moveClockValuation, indicesToClocks, "          \u2502  ");
    std::cout << "          \u2502\n";

    // Print the arena transition details.
    std::string actionName = arenaTransition.action.first;
    if (arenaTransition.action.second.has_value())
        actionName += in_out_act_to_string(arenaTransition.action.second.value());
    std::cout << "          \u251c\u2500 Action: " << actionName << "\n";
    if (!arenaTransition.clockGuard.empty())
    {
        std::cout << "          \u251c\u2500 Guard: ";
        for (size_t j = 0; j < arenaTransition.clockGuard.size(); j++)
        {
            if (j > 0)
                std::cout << " && ";
            std::cout << arenaTransition.clockGuard[j].to_string();
        }
        std::cout << "\n";
    }
    if (!arenaTransition.clocksToReset.empty())
    {
        std::cout << "          \u251c\u2500 Reset: ";
        for (size_t j = 0; j < arenaTransition.clocksToReset.size(); j++)
        {
            if (j > 0)
                std::cout << ", ";
            std::cout << arenaTransition.clocksToReset[j];
        }
        std::cout << "\n";
    }
    std::cout << "          \u2502\n";
    std::cout << "          \u25bc\n";
}


inline std::string region::StrategyGraph::escapeDot(const std::string &s)
{
    std::string result;
    result.reserve(s.size());

    for (const char c: s)
    {
        switch (c)
        {
            case '"':
                result += "\\\"";
                break;

            case '\\':
                result += "\\\\";
                break;

            default:
                result += c;
        }
    }

    return result;
}


std::string region::StrategyGraph::formatClockValuation(const clockValuation &cv,
                                                        const std::unordered_map<int, std::string> &indicesToClocks,
                                                        const std::string &separator)
{
    std::string result;

    for (int i = 0; i < static_cast<int>(cv.size()); i++)
    {
        if (i > 0)
            result += separator;
        result += indicesToClocks.at(i) + ": (" + std::to_string(cv[i].first) + ", " + (cv[i].second ? "frac > 0" : "frac = 0") + ")";
    }

    return result;
}


std::string region::StrategyGraph::dotRegionLabel(const Region &reg,
                                                  const std::unordered_map<int, std::string> &intToLocations,
                                                  const std::unordered_map<int, std::string> &indicesToClocks,
                                                  const absl::flat_hash_map<int, players_sym> &locationsToPlayers)
{
    const int regLocation = reg.getLocation();
    std::string label = escapeDot(intToLocations.at(regLocation) + " [" + players_sym_to_string(locationsToPlayers.at(regLocation)) + "]");

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto &cv = reg.getClockValuation();
    if (!cv.empty())
        label += "\n" + escapeDot(formatClockValuation(cv, indicesToClocks, "\n"));

    return label;
}


std::string region::StrategyGraph::dotEdgeLabel(const transition &arenaTransition,
                                                const clockValuation &moveCV,
                                                const std::unordered_map<int, std::string> &indicesToClocks)
{
    std::string label;

    // Move clock valuation (the clock values at the moment the discrete transition is taken).
    label += escapeDot(formatClockValuation(moveCV, indicesToClocks, "\n"));

    label += "\n\u2500\u2500\u2500";

    // Action name.
    std::string actionName = arenaTransition.action.first;
    if (arenaTransition.action.second.has_value())
        actionName += in_out_act_to_string(arenaTransition.action.second.value());
    label += "\\nAction: " + escapeDot(actionName);

    // Clock guard (only if non-empty).
    if (!arenaTransition.clockGuard.empty())
    {
        std::string guardStr;
        for (size_t i = 0; i < arenaTransition.clockGuard.size(); i++)
        {
            if (i > 0)
                guardStr += " && ";
            guardStr += arenaTransition.clockGuard[i].to_string();
        }
        label += "\\nGuard: " + escapeDot(guardStr);
    }

    // Clock resets (only if non-empty).
    if (!arenaTransition.clocksToReset.empty())
    {
        std::string resetStr;
        for (size_t i = 0; i < arenaTransition.clocksToReset.size(); i++)
        {
            if (i > 0)
                resetStr += ", ";
            resetStr += arenaTransition.clocksToReset[i];
        }
        label += "\\nReset: " + escapeDot(resetStr);
    }

    return label;
}


void region::StrategyGraph::to_dot(const std::string &path,
                                   const std::unordered_map<int, std::string> &indicesToClocks,
                                   const std::unordered_map<int, std::string> &intToLocations,
                                   const absl::flat_hash_map<int, players_sym> &locationsToPlayers) const
{
    std::ofstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file for .dot output: " + path);

    // Write DOT header.
    file << "digraph StrategyGraph {\n";
    file << "    rankdir=LR;\n";
    file << "    nodesep=3.0;\n";
    file << "    ranksep=7.0;\n";
    file << "    node [shape=box, fontname=\"Monospace:matrix=1 .1 0 1\"];\n\n";

    // Single BFS: assign IDs, write node declarations, and write edges in one traversal.
    // Assigning unique identifiers to regions, as this is required for DOT nodes representation.
    std::unordered_map<Region, int, RegionHash> regionToId;
    int nextId = 0;
    std::queue<RegionPtr> bfsQueue;

    // Used to enqueue region pointers during BFS exploration.
    const auto enqueue = [&](const Region &reg) -> RegionPtr
    {
        const auto [it, inserted] = regionToId.emplace(reg, nextId);

        if (inserted)
        {
            nextId++;

            // DOT-related formatting.
            const bool isTarget = targetRegions.contains(reg);
            const bool isHead = std::ranges::find(heads, reg) != heads.end();

            file << "    n" << it->second << " [label=\"" << dotRegionLabel(reg, intToLocations, indicesToClocks, locationsToPlayers) << "\"";

            if (isTarget)
                file << ", style=filled, fillcolor=lightgreen, peripheries=2";
            else if (isHead)
                file << ", style=filled, fillcolor=lightblue";

            file << "];\n";

            // Enqueuing the region pointer.
            bfsQueue.push(&it->first);
        }

        return &it->first;
    };

    for (const auto &head: heads)
        enqueue(head);

    // Invisible entry points: all head IDs are now assigned.
    file << "\n";
    for (int i = 0; i < static_cast<int>(heads.size()); i++)
    {
        file << "    __init" << i << " [shape=point, width=0.1];\n";
        file << "    __init" << i << " -> n" << regionToId.at(heads[i]) << ";\n";
    }
    file << "\n";

    const auto &allTransitions = getStrategyTransitions();

    while (!bfsQueue.empty())
    {
        const RegionPtr current = bfsQueue.front();
        bfsQueue.pop();

        if (const auto it = allTransitions.find(*current); it != allTransitions.end())
        {
            for (const auto &[arenaTransition, target, moveCV]: it->second)
            {
                const RegionPtr targetPtr = enqueue(target);
                file << "    n" << regionToId.at(*current) << " -> n" << regionToId.at(*targetPtr)
                        << " [label=\"" << dotEdgeLabel(arenaTransition, moveCV, indicesToClocks) << "\"];\n";
            }
        }
    }

    file << "}\n";
    file.close();
}


std::string region::StrategyGraph::to_string(const std::unordered_map<int, std::string> &intToLocations) const
{
    std::ostringstream oss;

    oss << "StrategyGraph:\n";

    oss << "  Heads (initial regions):\n";
    for (const auto &head: heads)
    {
        oss << "    [" << intToLocations.at(head.getLocation()) << " ";
        for (const auto &[h, hasNonZeroFrac]: head.getClockValuation())
            oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
        oss << "]\n";
    }

    oss << "\n\n  Target regions:\n";
    for (const auto &target: targetRegions)
    {
        oss << "    [" << intToLocations.at(target.getLocation()) << " ";
        for (const auto &[h, hasNonZeroFrac]: target.getClockValuation())
            oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
        oss << "]\n";
    }

    oss << "\n\n  Transitions:\n";
    for (const auto &[source, edges]: strategyTransitions)
    {
        oss << "    [" << intToLocations.at(source.getLocation()) << " ";
        for (const auto &[h, hasNonZeroFrac]: source.getClockValuation())
            oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
        oss << "]\n";
        for (const auto &[tr, target, cv]: edges)
        {
            oss << "      --[" << tr.to_string() << "]--> [" << intToLocations.at(target.getLocation()) << " ";
            for (const auto &[h, hasNonZeroFrac]: target.getClockValuation())
                oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
            oss << "] (predecessor cv: ";
            for (const auto &[h, hasNonZeroFrac]: cv)
                oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
            oss << ")\n";
        }
        oss << "\n";
    }

    return oss.str();
}
