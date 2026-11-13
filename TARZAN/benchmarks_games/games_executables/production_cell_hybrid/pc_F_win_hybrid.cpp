#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main(const int argc, char* argv[])
{

    const std::string arenaPath = "../../../TARZAN/benchmarks_games/models/production_cell/arena/production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    const std::string formulaPath = "../../../TARZAN/benchmarks_games/models/production_cell/winning_conditions/reachability1.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    region::RTSArena rts(arena, phi, false);

    const std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    if (startingRegions.size() > 1)
        std::exit(EXIT_FAILURE);

    // Selecting the reachability approach according to the flag argument "forward_reachability=true/false"
    exploration_approach reachabilityApproach = BACKWARD;

    if (argv[3] != nullptr)
    {
        std::cout << "\nAdditional parameters inserted." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (argc >= 2 && std::string(argv[1]) == "--forward_reachability")
    {
        auto param = (argv[2] != nullptr) ? std::string(argv[2]) : "error";

        if (param == "true")
        {
            reachabilityApproach = FORWARD;
        }
        else
        {
            if (param == "false")
            reachabilityApproach = BACKWARD;
            else
            {
                std::cout << "\nReachability approach is undefined." << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        if (argc >= 2)
        {
            std::cout << "\nReachability approach parameter is misspelled." << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    if (rts.solveTimedCLTLocGame(phi, reachabilityApproach))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;

    return 0;
}
