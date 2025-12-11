#include <string>
#include <algorithm>

#include "regions/networkOfTA/RTSNetwork.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/RTSArena.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/exceptions/nestedCLTLocFormula_exception.h"


inline void testArenaParsing()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/";
    const std::string arenaName = "arena0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(path + arenaName);

    std::cout << arena.to_string() << std::endl;

    const auto &locToIntMap = arena.mapLocationsToInt();

    std::cout << "Locations to int map: " << locToIntMap.size() << std::endl;
    for (const auto &[fst, snd]: locToIntMap)
        std::cout << fst << " - " << snd << std::endl;

    const auto &locToPlayerMap = arena.mapLocationsToPlayers(locToIntMap);

    std::cout << "Locations to player map: " << locToPlayerMap.size() << std::endl;
    for (const auto &[fst, snd]: locToPlayerMap)
        std::cout << fst << " - " << snd << std::endl;

    std::cout << "Clock indices: " << std::endl;
    for (const auto &[fst, snd]: arena.getClocksIndices())
        std::cout << fst << " - " << snd << std::endl;

    const region::RTSArena rts(arena);

    std::cout << rts.to_string() << std::endl;
}


inline void testCLTLocFormulaParsing()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/CLTLoc_formulae/formula0.txt";

    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(path);

    std::cout << phi.to_string() << std::endl;
}


inline void testCLTLocGetRegions()
{
    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/";
    const std::string arenaName = "arena0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    std::cout << arena.to_string() << std::endl;

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/CLTLoc_formulae/formula0.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);
    std::cout << phi.to_string() << std::endl;

    const region::RTSArena rts(arena, phi);
    std::cout << rts.to_string() << std::endl;

    try
    {
        std::vector<std::vector<region::Region>> res = rts.getRegionsFromGeneralCLTLocFormula(phi);
    } catch (const region::NestedCLTLocFormulaException &e)
    {
        std::cerr << "Invalid formula: " << e.what() << std::endl;
    }
}


void testRegionGeneration()
{
    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/";
    const std::string arenaName = "arena0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    std::cout << arena.to_string() << std::endl;

    std::vector<std::string> locations = { "q0", "q1" };

    const region::RTSArena rts(arena);

    std::vector<timed_automaton::ast::clockConstraint> clockConstraints = {

    };

    const std::vector<region::Region> regions = region::Region::generateRegionsFromConstraints(
        locations,
        clockConstraints,
        rts.getClocksIndices(),
        rts.getLocationsToInt(),
        rts.getMaxConstants(),
        static_cast<int>(rts.getClocksIndices().size()));

    for (const auto &reg: regions)
        std::cout << reg.toString() << "\n\n";
}


void testGetRegionsFromGeneralCLTLocFormula()
{
    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/";
    const std::string arenaName = "arena0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    std::cout << arena.to_string() << std::endl;

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/CLTLoc_formulae/formula1.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);
    std::cout << phi.to_string() << std::endl;

    const region::RTSArena rts(arena, phi);

    std::vector<std::vector<region::Region>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    std::cout << startingRegions.size() << std::endl;

    for (const auto &regVec : startingRegions)
    {
        std::cout << "===================\n";
        std::cout << regVec.size() << std::endl;
        for (const auto &reg: regVec)
            std::cout << reg.toString() << std::endl;
        std::cout << "===================\n";
    }
}


int main()
{
    testGetRegionsFromGeneralCLTLocFormula();

    return 0;
}
