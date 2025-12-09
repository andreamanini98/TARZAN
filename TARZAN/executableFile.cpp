#include <string>
#include <algorithm>

#include "regions/networkOfTA/RTSNetwork.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"


inline void testArenaParsing()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/";
    const std::string arenaName = "arena0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(path + arenaName);

    std::cout << arena.to_string() << std::endl;

    const auto &locToIntMap = arena.mapLocationsToInt();

    std::cout << "Locations to int map: " << locToIntMap.size() << std::endl;
    for (const auto &locToInt: locToIntMap)
        std::cout << locToInt.first << " - " << locToInt.second << std::endl;

    const auto &locToPlayerMap = arena.mapLocationsToPlayers(locToIntMap);

    std::cout << "Locations to player map: " << locToPlayerMap.size() << std::endl;
    for (const auto &locToPlayer: locToPlayerMap)
        std::cout << locToPlayer.first << " - " << locToPlayer.second << std::endl;

    std::cout << "Clock indices: " << std::endl;
    for (const auto &clockIndices: arena.getClocksIndices())
        std::cout << clockIndices.first << " - " << clockIndices.second << std::endl;

    const region::RTS rts(arena);

    std::cout << rts.to_string() << std::endl;
}


inline void testCLTLocFormulaParsing()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/CLTLoc_formulae/formula0.txt";

    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(path);

    std::cout << phi.to_string() << std::endl;
}


int main()
{
    testCLTLocFormulaParsing();

    return 0;
}
