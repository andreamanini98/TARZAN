#include <string>
#include <algorithm>
#include <chrono>

#ifdef _OPENMP
#include <omp.h>
#endif

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


void testOmegaFilter()
{
    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/";
    const std::string arenaName = "arena0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/CLTLoc_formulae/formula1.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const region::RTSArena rts(arena, phi);

    // std::cout << rts.to_string() << std::endl;

    std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    std::vector<RegionPtr> toProcess{};

    assert(startingRegions.size() > 1);

    auto &setG = startingRegions[1];

    // const auto &intersectionSet = startingRegions[0];

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    std::cout << "\n\n\n\nSTARTING SERIAL EXECUTION\n\n";

    std::cout << "Before omega filter: " << std::endl;
    std::cout << "setG size: " << setG.size() << std::endl;
    std::cout << "toProcess size: " << toProcess.size() << std::endl;

    std::unordered_set<region::Region, region::RegionHash> filteredRegions{};
    std::vector<RegionPtr> filteredRegionsPtr{};

    // Starting the timer for measuring computation.
    const auto start = std::chrono::high_resolution_clock::now();

    rts.omegaFilterSerial(setG, toProcess, filteredRegions, filteredRegionsPtr, {});

    // Ending the timer for measuring computation.
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Total time       : " << duration.count() << " microseconds." << std::endl;

    std::cout << "After omega filter: " << std::endl;
    std::cout << "setG size: " << setG.size() << std::endl;
    std::cout << "filteredRegions size: " << filteredRegions.size() << std::endl;
    std::cout << "toProcess size: " << toProcess.size() << std::endl;
    std::cout << "filteredRegionsPtr size: " << filteredRegionsPtr.size() << std::endl;

    //for (const auto &reg: setG)
    //    std::cout << reg.toString() << std::endl;
    //for (const auto &reg: filteredRegions)
    //    std::cout << reg.toString() << std::endl;

    std::cout << "\n\nSTARTING PARALLEL EXECUTION\n\n";

    std::cout << "Before omega filter: " << std::endl;
    std::cout << "setG size: " << setG.size() << std::endl;
    std::cout << "toProcess size: " << toProcess.size() << std::endl;

    std::unordered_set<region::Region, region::RegionHash> filteredRegions1{};
    std::vector<RegionPtr> filteredRegionsPtr1{};

    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start1 = omp_get_wtime();
#else
    const auto start1 = std::chrono::high_resolution_clock::now();
#endif

    rts.omegaFilter(setG, toProcess, filteredRegions1, filteredRegionsPtr1, {}, true);

    // Ending the timer for measuring computation.
#ifdef _OPENMP
    const auto end1 = omp_get_wtime();
    const auto duration1 = end1 - start1;
    std::cout << "Total time       : " << duration1 * 1000000 << " microseconds." << std::endl;
#else
    const auto end1 = std::chrono::high_resolution_clock::now();
    const auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << "Total time       : " << duration1 << " microseconds." << std::endl;
#endif

    std::cout << "After omega filter: " << std::endl;
    std::cout << "setG size: " << setG.size() << std::endl;
    std::cout << "filteredRegions1 size: " << filteredRegions1.size() << std::endl;
    std::cout << "toProcess size: " << toProcess.size() << std::endl;
    std::cout << "filteredRegionsPtr1 size: " << filteredRegionsPtr1.size() << std::endl;

    //for (const auto &reg: setG)
    //    std::cout << reg.toString() << std::endl;
    //for (const auto &reg: filteredRegions1)
    //    std::cout << reg.toString() << std::endl;

    std::unordered_set<region::Region, region::RegionHash> updatedSetG{};
    for (const auto &region: setG)
        updatedSetG.insert(region);
    for (const auto &region: filteredRegions)
        updatedSetG.insert(region);

    std::unordered_set<region::Region, region::RegionHash> updatedSetG1{};
    for (const auto &region: setG)
        updatedSetG1.insert(region);
    for (const auto &region: filteredRegions1)
        updatedSetG1.insert(region);

    if (updatedSetG == updatedSetG1)
        std::cout << "\nSets coincide!!!" << std::endl;
    else
        std::cout << "\nSets differ!!!";
}


void testDeltaFilter()
{
    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/";
    const std::string arenaName = "arena0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath + arenaName);

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/CLTLoc_formulae/formula2.txt";
    const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);

    const region::RTSArena rts(arena, phi);

    // std::cout << rts.to_string() << std::endl;

    // Starting the timer for measuring computation.
    const auto start0 = std::chrono::high_resolution_clock::now();

    std::vector<std::unordered_set<region::Region, region::RegionHash>> startingRegions = rts.getRegionsFromGeneralCLTLocFormula(phi);

    // Ending the timer for measuring computation.
    const auto end0 = std::chrono::high_resolution_clock::now();
    const auto duration0 = std::chrono::duration_cast<std::chrono::microseconds>(end0 - start0);
    std::cout << "\n\n\nTotal getRegionsFromGeneralCLTLocFormula time       : " << duration0.count() << " microseconds." << std::endl;

    std::vector<RegionPtr> toProcess{};

    assert(startingRegions.size() > 1);

    auto &setG = startingRegions[1];

    // const auto &intersectionSet = startingRegions[0];

    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    std::cout << "\n\n\n\nSTARTING SERIAL EXECUTION\n\n";

    std::cout << "Before delta filter: " << std::endl;
    std::cout << "setG size: " << setG.size() << std::endl;
    std::cout << "toProcess size: " << toProcess.size() << std::endl;

    std::unordered_set<region::Region, region::RegionHash> filteredRegions{};
    std::vector<RegionPtr> filteredRegionsPtr{};

    // Starting the timer for measuring computation.
    const auto start = std::chrono::high_resolution_clock::now();

    rts.deltaFilterSerial(setG, toProcess, filteredRegions, filteredRegionsPtr, {});

    // Ending the timer for measuring computation.
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Total time       : " << duration.count() << " microseconds." << std::endl;

    std::cout << "After delta filter: " << std::endl;
    std::cout << "setG size: " << setG.size() << std::endl;
    std::cout << "filteredRegions size: " << filteredRegions.size() << std::endl;
    std::cout << "toProcess size: " << toProcess.size() << std::endl;
    std::cout << "filteredRegionsPtr size: " << filteredRegionsPtr.size() << std::endl;

    //for (const auto &reg: setG)
    //    std::cout << reg.toString() << std::endl;
    //for (const auto &reg: filteredRegions)
    //    std::cout << reg.toString() << std::endl;

    std::cout << "\n\nSTARTING PARALLEL EXECUTION\n\n";

    std::cout << "Before delta filter: " << std::endl;
    std::cout << "setG size: " << setG.size() << std::endl;
    std::cout << "toProcess size: " << toProcess.size() << std::endl;

    std::unordered_set<region::Region, region::RegionHash> filteredRegions1{};
    std::vector<RegionPtr> filteredRegionsPtr1{};

    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start1 = omp_get_wtime();
#else
    const auto start1 = std::chrono::high_resolution_clock::now();
#endif

    rts.deltaFilter(setG, toProcess, filteredRegions1, filteredRegionsPtr1, {}, true);

    // Ending the timer for measuring computation.
#ifdef _OPENMP
    const auto end1 = omp_get_wtime();
    const auto duration1 = end1 - start1;
    std::cout << "Total time       : " << duration1 * 1000000 << " microseconds." << std::endl;
#else
    const auto end1 = std::chrono::high_resolution_clock::now();
    const auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << "Total time       : " << duration1 << " microseconds." << std::endl;
#endif

    std::cout << "After delta filter: " << std::endl;
    std::cout << "setG size: " << setG.size() << std::endl;
    std::cout << "filteredRegions1 size: " << filteredRegions1.size() << std::endl;
    std::cout << "toProcess size: " << toProcess.size() << std::endl;
    std::cout << "filteredRegionsPtr1 size: " << filteredRegionsPtr1.size() << std::endl;

    //for (const auto &reg: setG)
    //    std::cout << reg.toString() << std::endl;
    //for (const auto &reg: filteredRegions1)
    //    std::cout << reg.toString() << std::endl;

    std::unordered_set<region::Region, region::RegionHash> updatedSetG{};
    for (const auto &region: setG)
        updatedSetG.insert(region);
    for (const auto &region: filteredRegions)
        updatedSetG.insert(region);

    std::unordered_set<region::Region, region::RegionHash> updatedSetG1{};
    for (const auto &region: setG)
        updatedSetG1.insert(region);
    for (const auto &region: filteredRegions1)
        updatedSetG1.insert(region);

    std::cout << "UpdatedSetG1 size: " << updatedSetG1.size() << std::endl;

    if (updatedSetG == updatedSetG1)
        std::cout << "\nSets coincide!!!" << std::endl;
    else
        std::cout << "\nSets differ!!!";
}


int main()
{
    testOmegaFilter();

    testDeltaFilter();

    return 0;
}
