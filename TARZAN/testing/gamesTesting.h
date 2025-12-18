#ifndef TARZAN_GAMESTESTING_H
#define TARZAN_GAMESTESTING_H

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


inline void testRegionGeneration()
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


inline void testGetRegionsFromGeneralCLTLocFormula()
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


inline void testProductionCellWin()
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

    const auto &setG = startingRegions[0];
    std::vector<RegionPtr> toProcess{};

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    if (rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void testProductionCellLose()
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

    const auto &setG = startingRegions[0];
    std::vector<RegionPtr> toProcess{};

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    if (rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void testProductionCellSafetyWin()
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

    const auto &setG = startingRegions[0];
    std::vector<RegionPtr> toProcess{};

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    if (rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void testProductionCellSafetyLose()
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

    const auto &setG = startingRegions[0];
    std::vector<RegionPtr> toProcess{};

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    if (rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void testSolveTimedCLTLocGames()
{
    std::cout << "\nSolving general Timed CLTLoc Game." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/";
    const std::string arenaName = "production_cell.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/reachability2.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    if (const region::RTSArena rts(arena, phi); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void formula2()
{
    std::cout << "\nSolving the formula number 2 of section 6 of our paper. Should be winning for the controller." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/";
    const std::string arenaName = "production_cell_0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/until_0.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    if (const region::RTSArena rts(arena, phi); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void formula2_controllerCannotCycle()
{
    std::cout << "\nSolving the formula number 2 of section 6 of our paper." << std::endl;
    std::cout << "Here, we changed (fall_p, restore_p, [(zeno, >, 0), (x, >, 1), (x, <, 2)], [zeno], feed) to: " << std::endl;
    std::cout << "(fall_p, restore_p, [(zeno, >, 0), (x, >, 1), (x, <, 3)], [zeno], feed). The environment can choose a delay such that the" << std::endl;
    std::cout << "controller cannot then move from feed. It cannot cycle anymore to make time advance. Should be losing for the controller." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/";
    const std::string arenaName = "production_cell_1.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/until_1.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    if (const region::RTSArena rts(arena, phi); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void formula3_lose()
{
    std::cout << "\nSolving the formula number 3 of section 6 of our paper." << std::endl;
    std::cout << "The formula wrongly requires that x must be such that 1 <= x <= 3 until depot, but this cannot be, since x gets reset." << std::endl;
    std::cout << "Should be losing for the controller." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/";
    const std::string arenaName = "production_cell_2.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/until_2.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    if (const region::RTSArena rts(arena, phi); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void formula3_win()
{
    std::cout << "\nSolving the formula number 3 of section 6 of our paper." << std::endl;
    std::cout << "This is a corrected version requiring that 0 <= x <= 3 holds until depot. Should be winning for the controller." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/";
    const std::string arenaName = "production_cell_3.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/until_3.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    if (const region::RTSArena rts(arena, phi); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void formula2_controllerCanCycleOnlyInFall_d()
{
    std::cout << "\nSolving the formula number 2 of section 6 of our paper." << std::endl;
    std::cout << "Here, we changed (fall_p, restore_p, [(zeno, >, 0), (x, >, 1), (x, <, 2)], [zeno], feed) to: " << std::endl;
    std::cout << "(fall_p, restore_p, [(zeno, >, 0), (x, >, 1), (x, <, 3)], [zeno], feed), and" << std::endl;
    std::cout << "(fall_d, restore_d, [(zeno, >, 0), (x, >, 1), (x, <=, 5)], [zeno], feed) to: " << std::endl;
    std::cout << "(fall_d, restore_d, [(zeno, >, 0), (x, >, 1), (x, <, 2)], [zeno], feed). The controller is now unable to cycle in fall_p," << std::endl;
    std::cout << "but can do it fall_p. Should be winning for the controller." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/";
    const std::string arenaName = "production_cell_4.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/until_4.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    if (const region::RTSArena rts(arena, phi); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}


inline void formula2_controllerCanCycleOnlyInFall_d_lose()
{
    std::cout << "\nSolving the formula number 2 of section 6 of our paper." << std::endl;
    std::cout << "Here, we changed (fall_p, restore_p, [(zeno, >, 0), (x, >, 1), (x, <, 2)], [zeno], feed) to: " << std::endl;
    std::cout << "(fall_p, restore_p, [(zeno, >, 0), (x, >, 1), (x, <, 3)], [zeno], feed), and" << std::endl;
    std::cout << "(fall_d, restore_d, [(zeno, >, 0), (x, >, 1), (x, <=, 5)], [zeno], feed) to: " << std::endl;
    std::cout << "(fall_d, restore_d, [(zeno, >, 0), (x, >, 1), (x, <=, 2)], [zeno], feed). The controller is now unable to cycle in fall_p," << std::endl;
    std::cout << "but it can't cycle in fall_d either due to the constraint (x, <=, 2): recall that time must increase strictly monotonically" << std::endl;
    std::cout << "and the environment can block the game in feed if it chooses x == 2 in fall_d. Should be losing for the controller." << std::endl;

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/";
    const std::string arenaName = "production_cell_5.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/production_cell/correctness_tests/until_5.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    std::cout << phi.to_string() << std::endl;
    // std::cout << rts.to_string() << std::endl;

    if (const region::RTSArena rts(arena, phi); rts.solveTimedCLTLocGame(phi))
        std::cout << "-----" << std::endl;
    else
        std::cout << "----- " << std::endl;
}

#endif //TARZAN_GAMESTESTING_H
