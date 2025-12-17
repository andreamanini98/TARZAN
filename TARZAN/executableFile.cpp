#include <string>
#include <algorithm>
#include <chrono>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/RTSArena.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/exceptions/nestedCLTLocFormula_exception.h"


// A pointer to a region object.
using RegionPtr = const region::Region *;


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
        std::vector<std::unordered_set<region::Region, region::RegionHash>> res = rts.getRegionsFromGeneralCLTLocFormula(phi);
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

    const std::unordered_set<region::Region, region::RegionHash> regions = region::Region::generateRegionsFromConstraints(
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

    std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    std::cout << startingRegions.size() << std::endl;

    for (const auto &regVec: startingRegions)
    {
        std::cout << "===================\n";
        std::cout << regVec.size() << std::endl;
        for (const auto &reg: regVec)
            std::cout << reg.toString() << std::endl;
        std::cout << "===================\n";
    }
}


void testProductionCellWin()
{
    std::cout << "\nWinning reachability since we require that the production cell reaches depot in at most 11 time units." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/";
    const std::string arenaName = "production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/reachability1.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const region::RTSArena rts(arena, phi);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    if (startingRegions.size() > 1)
        std::exit(EXIT_FAILURE);

    auto &setG = startingRegions[0];
    std::vector<RegionPtr> toProcess{};

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    if (rts.timedReachability(setG, toProcess, 10000))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


void testProductionCellLose()
{
    std::cout << "\nLosing reachability since we require that the production cell reaches depot in at most 6 time units (it requires at least 11)." <<
            std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/";
    const std::string arenaName = "production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/reachability0.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const region::RTSArena rts(arena, phi);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    if (startingRegions.size() > 1)
        std::exit(EXIT_FAILURE);

    auto &setG = startingRegions[0];
    std::vector<RegionPtr> toProcess{};

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    if (rts.timedReachability(setG, toProcess, 10000))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


void testProductionCellSafetyWin()
{
    std::cout << "\nWinning safety since we require that the production cell never drops a plate." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/";
    const std::string arenaName = "production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/safety0.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const region::RTSArena rts(arena, phi);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    // std::exit(EXIT_SUCCESS);

    if (startingRegions.size() > 1)
        std::exit(EXIT_FAILURE);

    auto &setG = startingRegions[0];
    std::vector<RegionPtr> toProcess{};

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    if (rts.timedSafety(setG, toProcess, 10000))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


void testProductionCellSafetyLose()
{
    std::cout << "\nLosing safety since we require that the production cell never drops a plate and never visits the table location." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/";
    const std::string arenaName = "production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/safety1.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const region::RTSArena rts(arena, phi);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    // std::exit(EXIT_SUCCESS);

    if (startingRegions.size() > 1)
        std::exit(EXIT_FAILURE);

    auto &setG = startingRegions[0];
    std::vector<RegionPtr> toProcess{};

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    if (rts.timedSafety(setG, toProcess, 10000))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


void testSolveTimedCLTLocGames()
{
    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/";
    const std::string arenaName = "production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/reachability1.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const region::RTSArena rts(arena, phi);

    std::cout << phi.to_string() << std::endl;

    if (rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


int main()
{
    testProductionCellWin();
    // testProductionCellLose();
    // testProductionCellSafetyWin();
    // testProductionCellSafetyLose();

    testSolveTimedCLTLocGames();

    return 0;
}
