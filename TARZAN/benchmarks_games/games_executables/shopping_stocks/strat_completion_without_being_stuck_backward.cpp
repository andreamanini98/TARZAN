#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    const std::string arenaPath = "../../../TARZAN/benchmarks_games/models/shopping_stocks/arena/shopping_stocks.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    const std::string formulaPath = "../../../TARZAN/benchmarks_games/models/shopping_stocks/winning_conditions/reachability1.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    region::RTSArena rts(arena, phi, true);

    const std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    rts.strategyGraphToDot("../../../output/strategy_graphs/shopping_stocks/g1.dot", phi, BACKWARD);

    return 0;
}
