#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    const std::string arenaName = "autoclave_sterilization_system.txt";
    const std::string formulaName = "wc1.txt";

    const std::string formulaPath = "../../../TARZAN/benchmarks_games/models/autoclave_sterilization_system/winning_conditions/" + formulaName;
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const std::string arenaPath = "../../../TARZAN/benchmarks_games/models/autoclave_sterilization_system/arena/" + arenaName;
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);
 
    region::RTSArena rts(arena, phi, true);
    rts.strategyGraphToDot("../../../output/strategy_graphs/g1.dot", phi);

    return 0;
}
