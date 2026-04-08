#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // std::cout << "\nWinning safety since we require that the production cell never drops a plate." << std::endl;

    const std::string arenaPath = "../../TARZAN/benchmarks_games/models/production_cell/arena/production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    const std::string formulaPath = "../../TARZAN/benchmarks_games/models/production_cell/winning_conditions/safety0.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    region::RTSArena rts(arena, phi, false);

    // std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    // ReSharper disable once CppTooWideScopeInitStatement
    const std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    if (startingRegions.size() > 1)
        std::exit(EXIT_FAILURE);

    if (rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;

    return 0;
}
