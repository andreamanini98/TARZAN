#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/arena/production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    // ---

    // const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/winning_conditions/reachability1.txt";

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/winning_conditions/safety0.txt";

    // const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/winning_conditions/until6.txt";

    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    // ---

    // const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/winning_conditions/and_next.txt";

    // const cltloc::ast::conjunctionOfFormulae phi = TARZAN::parseConjunctionOfFormulae(formulaPath);

    region::RTSArena rts(arena, phi, true);

    std::cout << phi << std::endl;
    // std::cout << rts.to_string() << std::endl;

    rts.printPlay(phi);

    return 0;
}
