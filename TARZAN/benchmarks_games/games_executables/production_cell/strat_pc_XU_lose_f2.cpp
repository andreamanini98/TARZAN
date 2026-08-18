#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // std::cout << "\nSolving the formula number 2 of section 6 of our paper. Should be losing for the controller." << std::endl;

    const std::string arenaPath = "../../../TARZAN/benchmarks_games/models/production_cell/arena/production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    const std::string formulaPath = "../../../TARZAN/benchmarks_games/models/production_cell/winning_conditions/until3.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    // std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    region::RTSArena rts(arena, phi, true);
    rts.strategyGraphToDot("../../../output/strategy_graphs/g1.dot", phi);

    return 0;

    return 0;
}
