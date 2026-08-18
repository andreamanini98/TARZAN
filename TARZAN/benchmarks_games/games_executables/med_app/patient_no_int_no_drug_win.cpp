#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // Tbot can always complete a patient procedure without raising interrupts or making the drug expire.

    constexpr std::string arenaName = "med_app.txt";
    constexpr std::string formulaName = "safety1.txt";

    const std::string formulaPath = "../../../TARZAN/benchmarks_games/models/med_app/winning_conditions/" + formulaName;
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const std::string arenaPath = "../../../TARZAN/benchmarks_games/models/med_app/arena/" + arenaName;
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    if (region::RTSArena rts(arena, phi, false); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;

    return 0;
}
