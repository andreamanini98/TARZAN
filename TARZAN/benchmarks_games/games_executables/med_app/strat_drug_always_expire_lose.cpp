#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // Tbot always makes the drug expire (not proper safety).

    constexpr std::string arenaName = "med_app.txt";
    constexpr std::string formulaName = "safety0.txt";

    const std::string formulaPath = "../../../TARZAN/benchmarks_games/models/med_app/winning_conditions/" + formulaName;
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const std::string arenaPath = "../../../TARZAN/benchmarks_games/models/med_app/arena/" + arenaName;
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    region::RTSArena rts(arena, phi, true);
    rts.strategyGraphToDot("../../../output/strategy_graphs/g1.dot", phi);

    return 0;
}
