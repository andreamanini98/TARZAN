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
    std::cout << "\n\nTrainAHV93 Flat 2 explodes" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_02/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate3_controller0_train0_train3_cnt0");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", EQ, 4);
    intVarOrClockConstr.emplace_back("x2", EQ, 4);
    intVarOrClockConstr.emplace_back("y", EQ, 0);
    intVarOrClockConstr.emplace_back("z", EQ, 1);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, BFS, false);

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

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, BFS);
}


inline void testTrainAHV93Flat2Efficient()
{
    std::cout << "\n\nTrainAHV93 Flat 2 efficient" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_02/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate0_controller0_train0_train3_cnt0");
    std::cout << "Goal: " << goal << std::endl;

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", GT, 2);
    intVarOrClockConstr.emplace_back("x1", LT, 3);
    intVarOrClockConstr.emplace_back("x2", EQ, 4); // punctual
    intVarOrClockConstr.emplace_back("y", GT, 1);
    intVarOrClockConstr.emplace_back("y", LT, 2);
    intVarOrClockConstr.emplace_back("z", GT, 1); // unbounded

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, BFS, false);

    //std::exit(1);

    std::cout << "\n\n";

    constexpr int q = 572;
    const std::vector h = { 2, 4, 1, 1 };

    boost::dynamic_bitset<> unbounded0(4);
    unbounded0[0] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec{};
    unboundedVec.push_front(unbounded0);

    boost::dynamic_bitset<> bounded0(4);
    bounded0[3] = true;
    bounded0[1] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec0{};
    boundedVec0.push_front(bounded0);

    boost::dynamic_bitset<> bounded1(4);
    bounded1[3] = true;

    boost::dynamic_bitset<> bounded2(4);
    bounded2[1] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec1{};
    boundedVec1.push_front(bounded2);
    boundedVec1.push_front(bounded1);

    std::deque<boost::dynamic_bitset<>> boundedVec2{};
    boundedVec2.push_front(bounded1);
    boundedVec2.push_front(bounded2);

    boost::dynamic_bitset<> x0(4);
    x0[2] = true;

    region::Region reg0(q, h, unboundedVec, x0, boundedVec0, {});
    region::Region reg1(q, h, unboundedVec, x0, boundedVec1, {});
    region::Region reg2(q, h, unboundedVec, x0, boundedVec2, {});

    std::vector startingRegions = { reg0, reg1, reg2 };

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;
    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, BFS);
}


inline void testTrainAHV93Flat3Explodes()
{
    std::cout << "\n\nTrainAHV93 Flat 3 explodes" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_03/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Redirect cout to file
    // std::ofstream outFile(path + "rts_output.txt");
    // std::streambuf *coutBuffer = std::cout.rdbuf();
    // std::cout.rdbuf(outFile.rdbuf());

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    // Restore cout to terminal
    // std::cout.rdbuf(coutBuffer);

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate3_controller0_train0_train0_train3_cnt0");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", EQ, 4);
    intVarOrClockConstr.emplace_back("x2", EQ, 4);
    intVarOrClockConstr.emplace_back("x3", EQ, 4);
    intVarOrClockConstr.emplace_back("y", EQ, 0);
    intVarOrClockConstr.emplace_back("z", EQ, 1);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, BFS, false);

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

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;
    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, BFS);
}


inline void testTrainAHV93Flat3Efficient()
{
    std::cout << "\n\nTrainAHV93 Flat 3 efficient" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_03/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate0_controller0_train0_train0_train3_cnt0");
    std::cout << "Goal: " << goal << std::endl;

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", GT, 2);
    intVarOrClockConstr.emplace_back("x1", LT, 3);
    intVarOrClockConstr.emplace_back("x2", GT, 2);
    intVarOrClockConstr.emplace_back("x2", LT, 3);
    intVarOrClockConstr.emplace_back("x3", EQ, 4);
    intVarOrClockConstr.emplace_back("y", GT, 1);
    intVarOrClockConstr.emplace_back("y", LT, 2);
    intVarOrClockConstr.emplace_back("z", GT, 1);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, BFS, false);

    //std::exit(1);

    std::cout << "\n\n";

    constexpr int q = 3705;
    const std::vector h = { 2, 2, 4, 1, 1 };

    boost::dynamic_bitset<> unbounded0(5);
    unbounded0[0] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec{};
    unboundedVec.push_front(unbounded0);


    // Ordered partition 0: [[1], [3], [4]]
    boost::dynamic_bitset<> bounded0(5);
    bounded0[1] = true;
    boost::dynamic_bitset<> bounded1(5);
    bounded1[3] = true;
    boost::dynamic_bitset<> bounded2(5);
    bounded2[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec0;
    boundedVec0.push_front(bounded2);
    boundedVec0.push_front(bounded1);
    boundedVec0.push_front(bounded0);


    // Ordered partition 1: [[1], [4], [3]]
    boost::dynamic_bitset<> bounded3(5);
    bounded3[1] = true;
    boost::dynamic_bitset<> bounded4(5);
    bounded4[4] = true;
    boost::dynamic_bitset<> bounded5(5);
    bounded5[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec1;
    boundedVec1.push_front(bounded5);
    boundedVec1.push_front(bounded4);
    boundedVec1.push_front(bounded3);


    // Ordered partition 2: [[3], [1], [4]]
    boost::dynamic_bitset<> bounded6(5);
    bounded6[3] = true;
    boost::dynamic_bitset<> bounded7(5);
    bounded7[1] = true;
    boost::dynamic_bitset<> bounded8(5);
    bounded8[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec2;
    boundedVec2.push_front(bounded8);
    boundedVec2.push_front(bounded7);
    boundedVec2.push_front(bounded6);


    // Ordered partition 3: [[3], [4], [1]]
    boost::dynamic_bitset<> bounded9(5);
    bounded9[3] = true;
    boost::dynamic_bitset<> bounded10(5);
    bounded10[4] = true;
    boost::dynamic_bitset<> bounded11(5);
    bounded11[1] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec3;
    boundedVec3.push_front(bounded11);
    boundedVec3.push_front(bounded10);
    boundedVec3.push_front(bounded9);


    // Ordered partition 4: [[4], [1], [3]]
    boost::dynamic_bitset<> bounded12(5);
    bounded12[4] = true;
    boost::dynamic_bitset<> bounded13(5);
    bounded13[1] = true;
    boost::dynamic_bitset<> bounded14(5);
    bounded14[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec4;
    boundedVec4.push_front(bounded14);
    boundedVec4.push_front(bounded13);
    boundedVec4.push_front(bounded12);


    // Ordered partition 5: [[4], [3], [1]]
    boost::dynamic_bitset<> bounded15(5);
    bounded15[4] = true;
    boost::dynamic_bitset<> bounded16(5);
    bounded16[3] = true;
    boost::dynamic_bitset<> bounded17(5);
    bounded17[1] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec5;
    boundedVec5.push_front(bounded17);
    boundedVec5.push_front(bounded16);
    boundedVec5.push_front(bounded15);


    // Ordered partition 6: [[2, 3], [4]]
    boost::dynamic_bitset<> bounded18(5);
    bounded18[1] = true;
    bounded18[3] = true;
    boost::dynamic_bitset<> bounded19(5);
    bounded19[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec6;
    boundedVec6.push_front(bounded19);
    boundedVec6.push_front(bounded18);


    // Ordered partition 7: [[4], [2, 3]]
    boost::dynamic_bitset<> bounded20(5);
    bounded20[4] = true;
    boost::dynamic_bitset<> bounded21(5);
    bounded21[1] = true;
    bounded21[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec7;
    boundedVec7.push_front(bounded21);
    boundedVec7.push_front(bounded20);


    // Ordered partition 8: [[3], [2, 4]]
    boost::dynamic_bitset<> bounded22(5);
    bounded22[3] = true;
    boost::dynamic_bitset<> bounded23(5);
    bounded23[1] = true;
    bounded23[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec8;
    boundedVec8.push_front(bounded23);
    boundedVec8.push_front(bounded22);


    // Ordered partition 9: [[2, 4], [3]]
    boost::dynamic_bitset<> bounded24(5);
    bounded24[1] = true;
    bounded24[4] = true;
    boost::dynamic_bitset<> bounded25(5);
    bounded25[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec9;
    boundedVec9.push_front(bounded25);
    boundedVec9.push_front(bounded24);


    // Ordered partition 10: [[1], [3, 4]]
    boost::dynamic_bitset<> bounded26(5);
    bounded26[1] = true;
    boost::dynamic_bitset<> bounded27(5);
    bounded27[3] = true;
    bounded27[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec10;
    boundedVec10.push_front(bounded27);
    boundedVec10.push_front(bounded26);


    // Ordered partition 11: [[3, 4], [1]]
    boost::dynamic_bitset<> bounded28(5);
    bounded28[3] = true;
    bounded28[4] = true;
    boost::dynamic_bitset<> bounded29(5);
    bounded29[1] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec11;
    boundedVec11.push_front(bounded29);
    boundedVec11.push_front(bounded28);


    // Ordered partition 12: [[2, 3, 4]]
    boost::dynamic_bitset<> bounded30(5);
    bounded30[1] = true;
    bounded30[3] = true;
    bounded30[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec12;
    boundedVec12.push_front(bounded30);

    boost::dynamic_bitset<> x0(5);
    x0[2] = true;

    region::Region reg0(q, h, unboundedVec, x0, boundedVec0, {});
    region::Region reg1(q, h, unboundedVec, x0, boundedVec1, {});
    region::Region reg2(q, h, unboundedVec, x0, boundedVec2, {});
    region::Region reg3(q, h, unboundedVec, x0, boundedVec3, {});
    region::Region reg4(q, h, unboundedVec, x0, boundedVec4, {});
    region::Region reg5(q, h, unboundedVec, x0, boundedVec5, {});
    region::Region reg6(q, h, unboundedVec, x0, boundedVec6, {});
    region::Region reg7(q, h, unboundedVec, x0, boundedVec7, {});
    region::Region reg8(q, h, unboundedVec, x0, boundedVec8, {});
    region::Region reg9(q, h, unboundedVec, x0, boundedVec9, {});
    region::Region reg10(q, h, unboundedVec, x0, boundedVec10, {});
    region::Region reg11(q, h, unboundedVec, x0, boundedVec11, {});
    region::Region reg12(q, h, unboundedVec, x0, boundedVec12, {});

    std::vector startingRegions = { reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9, reg10, reg11, reg12 };

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;
    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, BFS);
}


inline void testTrainAHV93Flat2Reachable()
{
    std::cout << "\n\nTrainAHV93 Flat 2 reachable" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_02/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate2_controller2_train3_train3_cnt2");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, BFS, false);

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

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, BFS);
}


inline void testTrainAHV93Flat3Reachable()
{
    std::cout << "\n\nTrainAHV93 Flat 3 reachable" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarksFlat/trainAHV93Flat/tf_03/";
    const std::string automatonFileName = "Flatten.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Redirect cout to file
    // std::ofstream outFile(path + "rts_output.txt");
    // std::streambuf *coutBuffer = std::cout.rdbuf();
    // std::cout.rdbuf(outFile.rdbuf());

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    // Restore cout to terminal
    // std::cout.rdbuf(coutBuffer);

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("gate2_controller2_train3_train3_train3_cnt3");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, BFS, false);

    std::cout << rts[0].toString() << std::endl;

    //std::exit(1);

    // intero che rappresenta gate2_controller2_train3_train3_train3_cnt3: 775

    std::cout << "\n\n";

    constexpr int q = 775;
    const std::vector h = { 4, 3, 2, 2, 1 };

    boost::dynamic_bitset<> unbounded0(5);
    unbounded0[0] = true;

    boost::dynamic_bitset<> unbounded1(5);
    unbounded1[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec{};
    unboundedVec.push_front(unbounded0);
    unboundedVec.push_front(unbounded1);

    boost::dynamic_bitset<> bounded0(5);
    bounded0[2] = true;
    bounded0[3] = true;

    boost::dynamic_bitset<> bounded1(5);
    bounded1[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec{};
    boundedVec.push_front(bounded1);
    boundedVec.push_front(bounded0);

    boost::dynamic_bitset<> x0(5);

    region::Region reg0(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg0 };

    std::cout << "Starting from regions:" << std::endl;
    for (const auto &reg: startingRegions)
        std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    // It goes out of memory.
    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, BFS);
}

#endif //TARZAN_TRAINAHV93_BACKWARD_H
