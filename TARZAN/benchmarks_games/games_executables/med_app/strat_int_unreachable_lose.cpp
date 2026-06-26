#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // An interrupt state (before calling the doctor) is unreachable.
    // This is LOSING, meaning that Tbot cannot enforce reaching the interrupt state: it needs the intervention of humans.
    // In the paper, you can cite ethical decisions such as "the robot cannot take critical decisions unless a human approves".

    constexpr std::string arenaName = "med_app.txt";
    constexpr std::string formulaName = "q_int_unreachable.txt";

    const std::string formulaPath = "../../../TARZAN/benchmarks_games/models/med_app/winning_conditions/" + formulaName;
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const std::string arenaPath = "../../../TARZAN/benchmarks_games/models/med_app/arena/" + arenaName;
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    region::RTSArena rts(arena, phi, true);
    rts.strategyGraphToDot("../../../output/strategy_graphs/g1.dot", phi);

    return 0;
}
