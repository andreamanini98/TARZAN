#ifndef TARZAN_BRIDGE_BACKWARD_H
#define TARZAN_BRIDGE_BACKWARD_H

#include <string>
#include <fstream>

#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"


inline void testVikingsFlat2()
{
    std::cout << "\n\nVikings Flat 2" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/vikingsFlat/vf2/";
    const std::string automatonFileName = "vf.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "\n\n\n";
    std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("free_safe_safe_L1");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("y1", EQ, 20);
    intVarOrClockConstr.emplace_back("y2", EQ, 20);
    intVarOrClockConstr.emplace_back("z", GT, 0);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    std::cout << rts[0].toString();

    // intero che rappresenta free_safe_safe_L1: 38

    std::cout << "\n\n";

    constexpr int q = 38;
    const std::vector h = { 20, 20, 0 };

    boost::dynamic_bitset<> unbounded0(3);
    unbounded0[0] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec{};
    unboundedVec.push_front(unbounded0);

    std::deque<boost::dynamic_bitset<>> boundedVec{};

    boost::dynamic_bitset<> x0(3);
    x0[1] = true;
    x0[2] = true;

    region::Region reg0(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg0 };

    std::cout << "Starting from regions:" << std::endl;
    for (const auto &reg: startingRegions)
        std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


inline void testVikingsFlat3()
{
    std::cout << "\n\nVikings Flat 3" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/vikingsFlat/vf3/";
    const std::string automatonFileName = "vf.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "\n\n\n";
    std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("free_safe_safe_safe_L1");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("y1", EQ, 20);
    intVarOrClockConstr.emplace_back("y2", GT, 20);
    intVarOrClockConstr.emplace_back("y3", EQ, 20);
    intVarOrClockConstr.emplace_back("z", GT, 0);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    std::cout << rts[0].toString();

    // intero che rappresenta free_safe_safe_L1: 144

    std::cout << "\n\n";

    constexpr int q = 144;
    const std::vector h = { 20, 20, 20, 0 };

    boost::dynamic_bitset<> unbounded0(4);
    unbounded0[0] = true;

    boost::dynamic_bitset<> unbounded1(4);
    unbounded1[2] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec{};
    unboundedVec.push_front(unbounded1);
    unboundedVec.push_front(unbounded0);

    std::deque<boost::dynamic_bitset<>> boundedVec{};

    boost::dynamic_bitset<> x0(4);
    x0[1] = true;
    x0[3] = true;

    region::Region reg0(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg0 };

    std::cout << "Starting from regions:" << std::endl;
    for (const auto &reg: startingRegions)
        std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}

#endif //TARZAN_BRIDGE_BACKWARD_H
