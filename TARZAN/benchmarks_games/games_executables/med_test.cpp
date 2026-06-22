#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // std::cout << "\nSolving a conjunction of NEXT formulae. Should be winning for the controller." << std::endl;

    const std::string arenaPath = "../../TARZAN/benchmarks_games/models/med_app/arena/med_app.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    const std::string formulaPath = "../../TARZAN/benchmarks_games/models/med_app/winning_conditions/and_next2.txt";
    const cltloc::ast::conjunctionOfFormulae phi = TARZAN::parseConjunctionOfFormulae(formulaPath);

    // std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    if (region::RTSArena rts(arena, phi, false); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;

    return 0;
}
