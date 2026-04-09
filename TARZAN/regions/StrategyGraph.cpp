#include "StrategyGraph.h"

#include <sstream>


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
