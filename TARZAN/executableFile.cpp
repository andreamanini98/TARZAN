#include <string>

#include "regions/networkOfTA/RTSNetwork.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"


inline void testArenaParsing()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/";
    const std::string arenaName = "arena0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(path + arenaName);

    std::cout << arena.to_string() << std::endl;

    const auto &locToIntMap = arena.mapLocationsToInt();

    for (const auto &locToInt : locToIntMap)
        std::cout << locToInt.first << " - " << locToInt.second << std::endl;

    const auto &locToPlayerMap = arena.mapLocationsToPlayers(locToIntMap);

    for (const auto &locToPlayer : locToPlayerMap)
        std::cout << locToPlayer.first << " - " << locToPlayer.second << std::endl;
}


int main()
{
    testArenaParsing();

    return 0;
}
