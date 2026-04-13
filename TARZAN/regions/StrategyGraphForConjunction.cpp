#include "StrategyGraphForConjunction.h"

#include <sstream>


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
