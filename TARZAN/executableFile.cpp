#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // constexpr std::string formulaName = "reachability0.txt";
    // constexpr std::string formulaName = "reachability1.txt";
    // constexpr std::string formulaName = "safety0.txt";

    constexpr std::string formulaName = "and_next0.txt";

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/med_app/winning_conditions/" + formulaName;

    constexpr std::string arenaName = "med_app.txt";

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/med_app/arena/" + arenaName;
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    // const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const cltloc::ast::conjunctionOfFormulae phi = TARZAN::parseConjunctionOfFormulae(formulaPath);

    region::RTSArena rts(arena, phi, true);

    std::cout << phi << std::endl;
    // std::cout << rts.to_string() << std::endl;

    rts.strategyGraphToDot("/Users/echo/Desktop/PhD/Tools/TARZAN/output/strategy_graphs/g1.dot", phi);

    return 0;
}
