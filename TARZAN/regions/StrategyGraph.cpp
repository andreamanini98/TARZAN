#include "StrategyGraph.h"

#include <iostream>
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


void region::StrategyGraph::printClockValuationInStrategy(const std::vector<std::pair<int, bool>> &cv,
                                                          const std::unordered_map<int, std::string> &indicesToClocks,
                                                          const std::string &indent)
{
    for (int i = 0; i < static_cast<int>(cv.size()); i++)
        std::cout << indent << indicesToClocks.at(i) << " := (" << cv[i].first << ", " << (cv[i].second ? "frac > 0" : "frac = 0") << ")\n";
}


void region::StrategyGraph::printRegionInStrategy(const Region &reg,
                                                  const std::unordered_map<int, std::string> &intToLocations,
                                                  const std::unordered_map<int, std::string> &indicesToClocks,
                                                  const absl::flat_hash_map<int, players_sym> &locationsToPlayers,
                                                  const std::string &indent)
{
    const int regLocation = reg.getLocation();
    std::cout << indent << intToLocations.at(regLocation) << " [" << locationsToPlayers.at(regLocation) << "]\n";
    printClockValuationInStrategy(reg.getClockValuation(), indicesToClocks, indent);
}


inline void region::StrategyGraph::printRegionWithBox(const int step,
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
