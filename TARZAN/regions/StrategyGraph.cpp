#include "StrategyGraph.h"

#include <sstream>


void region::StrategyGraph::addStrategyTransition(const Region &sourceRegion, const std::string &action, const Region &targetRegion)
{
    transitions[sourceRegion].emplace(action, targetRegion);
}


transitionSet region::StrategyGraph::getTransitionsGivenSource(const Region &sourceRegion) const
{
    if (const auto it = transitions.find(sourceRegion); it != transitions.end())
        return it->second;
    return {};
}


std::string region::StrategyGraph::to_string() const
{
    std::ostringstream oss;

    oss << "StrategyGraph:\n";

    oss << "  Heads (initial regions):\n";
    for (const auto &head: heads)
        oss << "    " << head.toString() << "\n";

    oss << "  Target regions:\n";
    for (const auto &target: targetRegions)
        oss << "    " << target.toString() << "\n";

    oss << "  Transitions:\n";
    for (const auto &[source, edges]: transitions)
    {
        oss << source.toString();
        for (const auto &[action, target]: edges)
            oss << "      --[" << action << "]-->\n " << target.toString();

        oss << "\n\n";
    }

    return oss.str();
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
    for (const auto &[source, edges]: transitions)
    {
        oss << "    [" << intToLocations.at(source.getLocation()) << " ";
        for (const auto &[h, hasNonZeroFrac]: source.getClockValuation())
            oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
        oss << "]\n";
        for (const auto &[action, target]: edges)
        {
            oss << "      --[" << action << "]--> [" << intToLocations.at(target.getLocation()) << " ";
            for (const auto &[h, hasNonZeroFrac]: target.getClockValuation())
                oss << "(" << h << ", " << (hasNonZeroFrac ? "true" : "false") << ")";
            oss << "]\n";
        }
        oss << "\n";
    }

    return oss.str();
}
