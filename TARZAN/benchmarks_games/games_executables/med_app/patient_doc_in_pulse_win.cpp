#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // Tbot can complete a single patient procedure by calling the doctor during pulse measurement.

    constexpr std::string arenaName = "med_app.txt";
    constexpr std::string formulaName = "and_next0.txt";

    const std::string formulaPath = "../../../TARZAN/benchmarks_games/models/med_app/winning_conditions/" + formulaName;
    const cltloc::ast::conjunctionOfFormulae phi = TARZAN::parseConjunctionOfFormulae(formulaPath);

    const std::string arenaPath = "../../../TARZAN/benchmarks_games/models/med_app/arena/" + arenaName;
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    if (region::RTSArena rts(arena, phi, false); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;

    return 0;
}
