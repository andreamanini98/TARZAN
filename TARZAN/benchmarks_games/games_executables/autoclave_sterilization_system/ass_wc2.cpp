#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    const std::string arenaName = "autoclave_sterilization_system.txt";
    const std::string formulaName = "wc2.txt";

    const std::string formulaPath = "../../../TARZAN/benchmarks_games/models/autoclave_sterilization_system/winning_conditions/" + formulaName;
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const std::string arenaPath = "../../../TARZAN/benchmarks_games/models/autoclave_sterilization_system/arena/" + arenaName;
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    if (region::RTSArena rts(arena, phi, false); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;

    return 0;
}
