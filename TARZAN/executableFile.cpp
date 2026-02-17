#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // const std::string arenaPath = "../../TARZAN/benchmarks_games/models/production_cell/arena/production_cell.txt";
    // const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/CLTLoc_formulae/and_next.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    std::cout << phi << std::endl;

    return 0;
}
