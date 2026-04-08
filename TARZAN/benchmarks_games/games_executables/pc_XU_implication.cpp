#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // std::cout << "\nImplication testing." << std::endl;

    const std::string arenaPath = "../../TARZAN/benchmarks_games/models/production_cell/arena/production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    const std::string formulaPath = "../../TARZAN/benchmarks_games/models/production_cell/winning_conditions/until6.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    // std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    if (region::RTSArena rts(arena, phi, false); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;

    return 0;
}
