#include "StrategyGraphForConjunction.h"
#include "RTSArena.h"

#include <sstream>
#include <fstream>
#include <queue>
#include <unordered_set>


void region::StrategyGraphForConjunction::addStrategyTransition(const Region &source,
                                                                const transition &arenaTransition,
                                                                const Region &target,
                                                                const clockValuation &cv)
{
    if (strategyTransitionsForConjunction.empty())
        throw std::logic_error("strategyTransitionsForConjunction vector is empty!");

    strategyTransitionsForConjunction.back()[source].emplace(arenaTransition, target, cv);
}


void region::StrategyGraphForConjunction::addNewStrategyTransitionMapToBack()
{
    strategyTransitionsForConjunction.emplace_back();
}


strategyTransitionSet region::StrategyGraphForConjunction::getStrategyTransitionsGivenSourceAndIndex(const Region &source, const size_t index) const
{
    if (index >= strategyTransitionsForConjunction.size())
        throw std::logic_error("The index is too big!");

    if (const auto it = strategyTransitionsForConjunction[index].find(source); it != strategyTransitionsForConjunction[index].end())
        return it->second;
    return {};
}


// Deprecated function that generated a cyclic strategy graph. Keeping it only for comparisons.
/*void region::StrategyGraphForConjunction::to_dot(const std::string &path,
                                                 const std::unordered_map<int, std::string> &indicesToClocks,
                                                 const std::unordered_map<int, std::string> &intToLocations,
                                                 const absl::flat_hash_map<int, players_sym> &locationsToPlayers)
{
    for (const auto &map: strategyTransitionsForConjunction)
        for (const auto &[region, transSet]: map)
            strategyTransitions[region].insert(transSet.begin(), transSet.end());

    StrategyGraph::to_dot(path, indicesToClocks, intToLocations, locationsToPlayers);
}*/


void region::StrategyGraphForConjunction::to_dot(const std::string &path,
                                                 const std::unordered_map<int, std::string> &indicesToClocks,
                                                 const std::unordered_map<int, std::string> &intToLocations,
                                                 const absl::flat_hash_map<int, players_sym> &locationsToPlayers)
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

    const int totalMoves = static_cast<int>(strategyTransitionsForConjunction.size());

    std::unordered_map<Region, int, RegionHash> regionToId;
    int nextId = 0;

    // Visited keys are "regionId_displayLayer" since the same region can appear at different layers.
    // BFS queue carries (RegionPtr, arrayIdx) where arrayIdx indexes into strategyTransitionsForConjunction.
    std::unordered_set<std::string> visitedNodes;
    std::queue<std::pair<RegionPtr, int>> bfsQueue;

    // Used to enqueue region pointers during BFS exploration. Here we also require an index associated with a region.
    const auto enqueueNode = [&](const Region &reg, const int arrayIdx) -> RegionPtr
    {
        const auto [it, inserted] = regionToId.emplace(reg, nextId);

        if (inserted)
            nextId++;

        // Display layer = totalMoves - 1 - arrayIdx (heads are at arrayIdx = totalMoves - 1).
        // ReSharper disable once CppTooWideScopeInitStatement
        const int displayLayer = totalMoves - 1 - arrayIdx;

        // Here it->second is the integer ID assigned to a region.
        if (visitedNodes.insert(std::to_string(it->second) + "_" + std::to_string(displayLayer)).second)
        {
            // We emplace regions even when arrayIdx is 0, since the targets of such regions will call enqueueNode as well (with arrayIdx = -1).
            if (arrayIdx >= 0)
                bfsQueue.emplace(&it->first, arrayIdx);

            const std::string suffix = "_" + std::to_string(displayLayer);

            // DOT-related formatting.
            const bool isTarget = targetRegions.contains(reg);
            const bool isHead = std::ranges::find(heads, reg) != heads.end();

            file << "    n" << it->second << suffix << " [label=\""
                    << dotRegionLabel(reg, suffix, intToLocations, indicesToClocks, locationsToPlayers) << "\"";

            // When arrayIdx == -1 is true, the corresponding region is a target region only if no additional moves can generate from such region.
            // When arrayIdx == totalMoves - 1 is true, the corresponding region is an initial region (in heads).
            if (isTarget && arrayIdx == -1)
                file << ", style=filled, fillcolor=lightgreen, peripheries=2";
            else if (isHead && arrayIdx == totalMoves - 1)
                file << ", style=filled, fillcolor=lightblue";

            file << "];\n";
        }

        return &it->first;
    };

    for (const auto &head: heads)
        enqueueNode(head, totalMoves - 1);

    // Invisible entry points: all head IDs are now assigned.
    file << "\n";
    for (int i = 0; i < static_cast<int>(heads.size()); i++)
    {
        file << "    __init" << i << " [shape=point, width=0.1];\n";
        file << "    __init" << i << " -> n" << regionToId.at(heads[i]) << "_0;\n";
    }
    file << "\n";

    while (!bfsQueue.empty())
    {
        const auto [current, arrayIdx] = bfsQueue.front();
        bfsQueue.pop();

        const int nextArrayIdx = arrayIdx - 1;

        if (const auto it = strategyTransitionsForConjunction[arrayIdx].find(*current); it != strategyTransitionsForConjunction[arrayIdx].end())
        {
            const std::string srcSuffix = "_" + std::to_string(totalMoves - 1 - arrayIdx);

            for (const auto &[arenaTransition, target, moveCV]: it->second)
            {
                const RegionPtr targetPtr = enqueueNode(target, nextArrayIdx);

                file << "    n" << regionToId.at(*current) << srcSuffix << " -> n" << regionToId.at(*targetPtr) << "_" << (totalMoves - 1 - nextArrayIdx)
                        << " [label=\"" << dotEdgeLabel(arenaTransition, moveCV, indicesToClocks) << "\"];\n";
            }
        }
    }

    file << "}\n";
    file.close();
}


std::string region::StrategyGraphForConjunction::to_string(const std::unordered_map<int, std::string> &intToLocations) const
{
    std::ostringstream oss;

    oss << StrategyGraph::to_string(intToLocations);

    oss << "\n  Conjunction transition layers (the i-th layer corresponds to the i-th element of strategyTransitionsForConjunction):\n";
    for (size_t i = 0; i < strategyTransitionsForConjunction.size(); i++)
    {
        oss << "    Layer " << i << ":\n";
        for (const auto &[source, edges]: strategyTransitionsForConjunction[i])
        {
            oss << "      [" << intToLocations.at(source.getLocation()) << " ";
            for (const auto &[h, hasNonZeroFrac]: source.getClockValuation())
                oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
            oss << "]\n";
            for (const auto &[tr, target, cv]: edges)
            {
                oss << "        --[" << tr.to_string() << "]--> [" << intToLocations.at(target.getLocation()) << " ";
                for (const auto &[h, hasNonZeroFrac]: target.getClockValuation())
                    oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
                oss << "] (predecessor cv: ";
                for (const auto &[h, hasNonZeroFrac]: cv)
                    oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
                oss << ")\n";
            }
            oss << "\n";
        }
    }

    return oss.str();
}
