#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // Arena.
    // const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/arena/production_cell.txt";
    // const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    // Reachability.
    // const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/winning_conditions/reachability1.txt";

    // Safety.
    // const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/winning_conditions/safety0.txt";

    // Next Until.
    // const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/winning_conditions/until6.txt";

    // Formula parser.
    // const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    // ---

    // Arena.
    // const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/arena/production_cell.txt";
    // const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    // And Next acyclic.
    // const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/production_cell/winning_conditions/and_next.txt";

    // Formula parser.
    // const cltloc::ast::conjunctionOfFormulae phi = TARZAN::parseConjunctionOfFormulae(formulaPath);

    // ---

    // Arena.
    // const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/and_next_strategies/arenas/production_cell_cyclic.txt";
    // const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    // And Next cyclic.
    // const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/and_next_strategies/winning_conditions/and_next_cyclic.txt";

    // Formula parser.
    // const cltloc::ast::conjunctionOfFormulae phi = TARZAN::parseConjunctionOfFormulae(formulaPath);

    // ---

    // Arena.
    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/and_next_strategies/arenas/production_cell_cyclic_short.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    // And Next cyclic short.
    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/and_next_strategies/winning_conditions/and_next_cyclic_short";

    // Formula parser.
    const cltloc::ast::conjunctionOfFormulae phi = TARZAN::parseConjunctionOfFormulae(formulaPath);

    // ---

    region::RTSArena rts(arena, phi, true);

    std::cout << phi << std::endl;
    // std::cout << rts.to_string() << std::endl;

    rts.strategyGraphToDot("/Users/echo/Desktop/PhD/Tools/TARZAN/output/strategy_graphs/g1.dot", phi);

    return 0;
}
