#ifndef TARZAN_TRAINAHV93_BACKWARD_H
#define TARZAN_TRAINAHV93_BACKWARD_H

#include <string>
#include <fstream>

#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"


inline void testTrainAHV93Flat2Explodes()
{
    std::cout << "\n\nTrainAHV93 Flat 2" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_02/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "\n\n\n";
    std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate3_controller0_train0_train3_cnt0");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", EQ, 4);
    intVarOrClockConstr.emplace_back("x2", EQ, 4);
    intVarOrClockConstr.emplace_back("y", EQ, 0);
    intVarOrClockConstr.emplace_back("z", EQ, 1);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    // intero che rappresenta gate3_controller0_train0_train3_cnt0: 89

    //std::exit(1);

    std::cout << "\n\n";

    constexpr int q = 89;
    const std::vector h = { 4, 4, 0, 1 };

    std::deque<boost::dynamic_bitset<>> unboundedVec{};

    std::deque<boost::dynamic_bitset<>> boundedVec{};

    boost::dynamic_bitset<> x0(4);
    x0[0] = true;
    x0[1] = true;
    x0[2] = true;
    x0[3] = true;

    region::Region reg0(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg0 };

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


inline void testTrainAHV93Flat2Efficient()
{
    std::cout << "\n\nTrainAHV93 Flat 2" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_02/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "\n\n\n";
    std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate3_controller0_train0_train3_cnt1");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", EQ, 4);
    intVarOrClockConstr.emplace_back("x2", EQ, 4);
    intVarOrClockConstr.emplace_back("y", EQ, 0);
    intVarOrClockConstr.emplace_back("z", EQ, 1);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    // intero che rappresenta gate3_controller0_train0_train3_cnt1: 88

    //std::exit(1);

    std::cout << "\n\n";

    constexpr int q = 88;
    const std::vector h = { 4, 4, 0, 1 };

    std::deque<boost::dynamic_bitset<>> unboundedVec{};

    std::deque<boost::dynamic_bitset<>> boundedVec{};

    boost::dynamic_bitset<> x0(4);
    x0[0] = true;
    x0[1] = true;
    x0[2] = true;
    x0[3] = true;

    region::Region reg0(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg0 };

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


inline void testTrainAHV93Flat3Explodes()
{
    std::cout << "\n\nTrainAHV93 Flat 3" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_03/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Redirect cout to file
    std::ofstream outFile(path + "rts_output.txt");
    std::streambuf *coutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(outFile.rdbuf());

    std::cout << "\n\n\n";
    std::cout << regionTransitionSystem.to_string() << std::endl;

    // Restore cout to terminal
    std::cout.rdbuf(coutBuffer);

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate3_controller0_train0_train0_train3_cnt0");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", EQ, 4);
    intVarOrClockConstr.emplace_back("x2", EQ, 4);
    intVarOrClockConstr.emplace_back("x3", EQ, 4);
    intVarOrClockConstr.emplace_back("y", EQ, 0);
    intVarOrClockConstr.emplace_back("z", EQ, 1);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    // intero che rappresenta gate3_controller0_train0_train0_train3_cnt0: 972

    //std::exit(1);

    std::cout << "\n\n";

    constexpr int q = 972;
    const std::vector h = { 4, 4, 4, 0, 1 };

    std::deque<boost::dynamic_bitset<>> unboundedVec{};

    std::deque<boost::dynamic_bitset<>> boundedVec{};

    boost::dynamic_bitset<> x0(5);
    x0[0] = true;
    x0[1] = true;
    x0[2] = true;
    x0[3] = true;
    x0[4] = true;

    region::Region reg0(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg0 };

    std::cout << "Starting from regions:" << std::endl;
    for (const auto &reg: startingRegions)
        std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;
    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


inline void testTrainAHV93Flat3Efficient()
{
    std::cout << "\n\nTrainAHV93 Flat 3" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_03/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Redirect cout to file
    std::ofstream outFile(path + "rts_output.txt");
    std::streambuf *coutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(outFile.rdbuf());

    std::cout << "\n\n\n";
    std::cout << regionTransitionSystem.to_string() << std::endl;

    // Restore cout to terminal
    std::cout.rdbuf(coutBuffer);

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate3_controller0_train0_train0_train3_cnt1");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", EQ, 4);
    intVarOrClockConstr.emplace_back("x2", EQ, 4);
    intVarOrClockConstr.emplace_back("x3", EQ, 4);
    intVarOrClockConstr.emplace_back("y", EQ, 0);
    intVarOrClockConstr.emplace_back("z", EQ, 1);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    // intero che rappresenta gate3_controller0_train0_train0_train3_cnt0: 972

    //std::exit(1);

    std::cout << "\n\n";

    constexpr int q = 2200;
    const std::vector h = { 4, 4, 4, 0, 1 };

    std::deque<boost::dynamic_bitset<>> unboundedVec{};

    std::deque<boost::dynamic_bitset<>> boundedVec{};

    boost::dynamic_bitset<> x0(5);
    x0[0] = true;
    x0[1] = true;
    x0[2] = true;
    x0[3] = true;
    x0[4] = true;

    region::Region reg0(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg0 };

    std::cout << "Starting from regions:" << std::endl;
    for (const auto &reg: startingRegions)
        std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;
    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


inline void testTrainAHV93Flat2Reachable()
{
    std::cout << "\n\nTrainAHV93 Flat 2" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_02/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "\n\n\n";
    std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate2_controller2_train3_train3_cnt2");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    std::cout << rts[0].toString() << std::endl;

    // intero che rappresenta gate2_controller2_train3_train3_cnt2: 714

    std::cout << "\n\n";

    constexpr int q = 714;
    const std::vector h = { 3, 2, 2, 1 };

    boost::dynamic_bitset<> unbounded0(4);
    unbounded0[0] = true;

    boost::dynamic_bitset<> unbounded1(4);
    unbounded1[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec{};
    unboundedVec.push_front(unbounded0);
    unboundedVec.push_front(unbounded1);

    boost::dynamic_bitset<> bounded0(4);
    bounded0[2] = true;
    bounded0[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec{};
    boundedVec.push_front(bounded0);

    boost::dynamic_bitset<> x0(4);

    region::Region reg0(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg0 };

    std::cout << "Starting from regions:" << std::endl;
    for (const auto &reg: startingRegions)
        std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}

#endif //TARZAN_TRAINAHV93_BACKWARD_H
