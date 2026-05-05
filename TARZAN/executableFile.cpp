#include <iostream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/RTSArena.h"


int main()
{
    // constexpr std::string formulaName = "reachability0.txt"; // Tbot can complete a single patient procedure.
    // constexpr std::string formulaName = "reachability1.txt"; // Tbot can complete a single patient procedure in at most 50 time units.
    // constexpr std::string formulaName = "safety0.txt";       // Tbot always makes the drug expire (not proper safety).
    // constexpr std::string formulaName = "safety1.txt";          // Tbot can always complete a patient procedure without raising interrupts or making the drug expire.

    // constexpr std::string formulaName = "and_next0.txt"; // Tbot can complete a single patient procedure by calling the nurse during pulse measurement.
    constexpr std::string formulaName = "and_next1.txt"; // Tbot can complete a single patient procedure by raising an interrupt during pulse measurement
    //                                                      three times, after which a nurse is called. The nurse intervention makes Tbot complete without
    //                                                      raising interrupts or requiring safety mode.

    const std::string formulaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/med_app/winning_conditions/" + formulaName;

    // const cltloc::ast::generalCLTLocFormula phi = TARZAN::parseGeneralCLTLocFormula(formulaPath);
    const cltloc::ast::conjunctionOfFormulae phi = TARZAN::parseConjunctionOfFormulae(formulaPath);

    // ---

    constexpr std::string arenaName = "med_app.txt";

    const std::string arenaPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks_games/models/med_app/arena/" + arenaName;
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(arenaPath);

    // ---

    region::RTSArena rts(arena, phi, true);

    std::cout << phi << std::endl;
    // std::cout << rts.to_string() << std::endl;

    rts.strategyGraphToDot("/Users/echo/Desktop/PhD/Tools/TARZAN/output/strategy_graphs/g1.dot", phi);

    return 0;
}
