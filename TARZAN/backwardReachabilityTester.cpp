#include <string>
#include <fstream>

#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"


inline void test0()
{
    std::cout << "Test 0 (Flower04): total number of region of the forward explored state space.\n";
    std::cout <<
            "Notice that when going backwards, the total number of regions may be greater than forward exploration, since when going backwards we use the\n";
    std::cout << "additional information about the ordering in which clock became unbounded, thus producing a finer partitioning of the state space.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_04/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(10, DFS);

    std::cout << '\n';
}


inline void test1()
{
    std::cout << "Test 1 (Flower04): showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_04/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void test2()
{
    std::cout << "Test 2 (Flower04): showing that from a reachable region it is possible to reach an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_04/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Suppress forwardReachability output
    std::streambuf *oldCoutBuffer = std::cout.rdbuf();
    std::ofstream nullStream("/dev/null");
    std::cout.rdbuf(nullStream.rdbuf());

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    // Restore cout
    std::cout.rdbuf(oldCoutBuffer);

    const std::vector new_h = { 1, 2, 0, 0, 1 };
    rts[0].set_h(new_h);
    rts[0].set_q(1);

    boost::dynamic_bitset<> newUnbounded(5);
    newUnbounded[0] = true;

    std::deque<boost::dynamic_bitset<>> newUnboundedVec;
    newUnboundedVec.push_front(newUnbounded);
    rts[0].set_unbounded(newUnboundedVec);

    boost::dynamic_bitset<> newX0(5);
    newX0[4] = true;
    newX0[3] = true;
    newX0[2] = true;
    newX0[1] = true;
    rts[0].set_x0(newX0);

    // This modified region is actually reachable since transitions must fire one at a time.
    // When clocks have integer value 1, 2, 3, 4, 1 (y is unbounded), it is possible to reset x3 and x4 to get this starting region.
    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void test3()
{
    std::cout << "Test 3 (Flower04): showing that from an unreachable region it is not possible to reach an initial region.\n";
    std::cout << "In this case, the region is unreachable since it assumes that clock y goes unbounded simultaneously with clock x2.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_04/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Suppress forwardReachability output
    std::streambuf *oldCoutBuffer = std::cout.rdbuf();
    std::ofstream nullStream("/dev/null");
    std::cout.rdbuf(nullStream.rdbuf());

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    // Restore cout
    std::cout.rdbuf(oldCoutBuffer);

    const std::vector new_h = { 1, 2, 0, 0, 1 };
    rts[0].set_h(new_h);
    rts[0].set_q(1);

    boost::dynamic_bitset<> newUnbounded(5);
    newUnbounded[0] = true;
    newUnbounded[3] = true;

    std::deque<boost::dynamic_bitset<>> newUnboundedVec;
    newUnboundedVec.push_front(newUnbounded);
    rts[0].set_unbounded(newUnboundedVec);

    boost::dynamic_bitset<> newX0(5);
    newX0[4] = true;
    newX0[2] = true;
    newX0[1] = true;
    rts[0].set_x0(newX0);

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


void test4()
{
    std::cout << "Test 4 (Flower04): showing that from a reachable region it is possible to reach an initial region.\n";
    std::cout << "In this case, the region is reachable since clock y goes unbounded before clock x2.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_04/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Suppress forwardReachability output
    std::streambuf *oldCoutBuffer = std::cout.rdbuf();
    std::ofstream nullStream("/dev/null");
    std::cout.rdbuf(nullStream.rdbuf());

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    // Restore cout
    std::cout.rdbuf(oldCoutBuffer);

    const std::vector new_h = { 1, 2, 0, 0, 1 };
    rts[0].set_h(new_h);
    rts[0].set_q(1);

    boost::dynamic_bitset<> newUnboundedY(5);
    newUnboundedY[0] = true;

    boost::dynamic_bitset<> newUnboundedX2(5);
    newUnboundedX2[3] = true;

    std::deque<boost::dynamic_bitset<>> newUnboundedVec;
    newUnboundedVec.push_front(newUnboundedY);
    newUnboundedVec.push_front(newUnboundedX2);
    rts[0].set_unbounded(newUnboundedVec);

    boost::dynamic_bitset<> newX0(5);
    newX0[4] = true;
    newX0[2] = true;
    newX0[1] = true;
    rts[0].set_x0(newX0);

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


void test5()
{
    std::cout << "Test 5 (Flower04): showing that from a region with an invalid location it is not possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_04/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Suppress forwardReachability output
    std::streambuf *oldCoutBuffer = std::cout.rdbuf();
    std::ofstream nullStream("/dev/null");
    std::cout.rdbuf(nullStream.rdbuf());

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    // Restore cout
    std::cout.rdbuf(oldCoutBuffer);

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    rts[0].set_q(10);

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


void test6()
{
    std::cout << "Test 6 (exSITH): showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/exSITH/liana/exSITH/";

    const std::string automatonFileName = "exSITH.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(2, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void test7()
{
    std::cout << "Test 7 (simple1000): showing that from the region reached forward it is possible to go back to an initial region.\n";
    std::cout << "Recall that in backward reachability integer variables are not supported. In this case, the variable i does not affect\n";
    std::cout << "the execution of the automaton, hence we are able to perform backward reachability.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/simple/liana/simple_1000/";

    const std::string automatonFileName = "Template.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


void testFischerFlat2()
{
    std::cout << "\n\nFischer Flat 2" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/fischerFlat/ff_02/";
    const std::string automatonFileName = "ff.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("cs1_cs2_id2");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", GT, 2);
    intVarOrClockConstr.emplace_back("x2", GT, 2);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    // intero che rappresenta cs1_cs2_id2: 0

    std::cout << "\n\n";

    constexpr int q = 0;
    const std::vector h = { 2, 2 };

    boost::dynamic_bitset<> unbounded0(2);
    boost::dynamic_bitset<> unbounded1(2);
    unbounded0[0] = true;
    unbounded1[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec0;
    unboundedVec0.push_front(unbounded0);
    unboundedVec0.push_front(unbounded1);

    std::deque<boost::dynamic_bitset<>> unboundedVec1;
    unboundedVec1.push_front(unbounded1);
    unboundedVec1.push_front(unbounded0);

    boost::dynamic_bitset<> unbounded2(2);
    unbounded2[0] = true;
    unbounded2[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec2;
    unboundedVec2.push_front(unbounded2);

    std::deque<boost::dynamic_bitset<>> boundedVec{};

    boost::dynamic_bitset<> x0(2);

    region::Region reg0(q, h, unboundedVec0, x0, boundedVec, {});
    region::Region reg1(q, h, unboundedVec1, x0, boundedVec, {});
    region::Region reg2(q, h, unboundedVec2, x0, boundedVec, {});

    std::vector startingRegions = { reg0, reg1, reg2 };

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


void testFischerFlat3()
{
    std::cout << "\n\nFischer Flat 3" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/fischerFlat/ff_03/";
    const std::string automatonFileName = "ff.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("req1_cs2_cs3_id3");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", GT, 1);
    intVarOrClockConstr.emplace_back("x1", LT, 2);
    intVarOrClockConstr.emplace_back("x2", GT, 2);
    intVarOrClockConstr.emplace_back("x3", GT, 2);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    // intero che rappresenta req1_cs2_cs3_id3: 85

    std::cout << "\n\n";

    constexpr int q = 85;
    const std::vector h = { 1, 2, 2 };

    boost::dynamic_bitset<> unbounded0(3);
    boost::dynamic_bitset<> unbounded1(3);
    unbounded0[0] = true;
    unbounded1[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec0;
    unboundedVec0.push_front(unbounded0);
    unboundedVec0.push_front(unbounded1);

    std::deque<boost::dynamic_bitset<>> unboundedVec1;
    unboundedVec1.push_front(unbounded1);
    unboundedVec1.push_front(unbounded0);

    boost::dynamic_bitset<> unbounded2(3);
    unbounded2[0] = true;
    unbounded2[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec2;
    unboundedVec2.push_front(unbounded2);

    boost::dynamic_bitset<> bounded0(3);
    bounded0[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec;
    boundedVec.push_front(bounded0);

    boost::dynamic_bitset<> x0(3);

    region::Region reg0(q, h, unboundedVec0, x0, boundedVec, {});
    region::Region reg1(q, h, unboundedVec1, x0, boundedVec, {});
    region::Region reg2(q, h, unboundedVec2, x0, boundedVec, {});

    std::vector startingRegions = { reg0, reg1, reg2 };

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


void testFischerFlat4()
{
    std::cout << "\n\nFischer Flat 4" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/fischerFlat/ff_04/";
    const std::string automatonFileName = "ff.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("req1_req2_cs3_cs4_id4");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", GT, 1);
    intVarOrClockConstr.emplace_back("x1", LT, 2);
    intVarOrClockConstr.emplace_back("x2", GT, 1);
    intVarOrClockConstr.emplace_back("x2", LT, 2);
    intVarOrClockConstr.emplace_back("x3", GT, 2);
    intVarOrClockConstr.emplace_back("x4", GT, 2);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    // intero che rappresenta req1_req2_cs3_cs4_id4: 571

    std::cout << "\n\n";

    constexpr int q = 571;
    const std::vector h = { 1, 1, 2, 2 };

    boost::dynamic_bitset<> unbounded0(4);
    boost::dynamic_bitset<> unbounded1(4);
    unbounded0[0] = true;
    unbounded1[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec0;
    unboundedVec0.push_front(unbounded0);
    unboundedVec0.push_front(unbounded1);

    std::deque<boost::dynamic_bitset<>> unboundedVec1;
    unboundedVec1.push_front(unbounded1);
    unboundedVec1.push_front(unbounded0);

    boost::dynamic_bitset<> unbounded2(4);
    unbounded2[0] = true;
    unbounded2[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec2;
    unboundedVec2.push_front(unbounded2);

    boost::dynamic_bitset<> bounded0(4);
    bounded0[2] = true;
    bounded0[3] = true;

    boost::dynamic_bitset<> bounded1(4);
    bounded1[2] = true;

    boost::dynamic_bitset<> bounded2(4);
    bounded2[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec0;
    boundedVec0.push_front(bounded0);

    std::deque<boost::dynamic_bitset<>> boundedVec1;
    boundedVec1.push_front(bounded1);
    boundedVec1.push_front(bounded2);

    std::deque<boost::dynamic_bitset<>> boundedVec2;
    boundedVec2.push_front(bounded2);
    boundedVec2.push_front(bounded1);

    boost::dynamic_bitset<> x0(4);

    region::Region reg0(q, h, unboundedVec0, x0, boundedVec0, {});
    region::Region reg1(q, h, unboundedVec0, x0, boundedVec1, {});
    region::Region reg2(q, h, unboundedVec0, x0, boundedVec2, {});
    region::Region reg3(q, h, unboundedVec1, x0, boundedVec0, {});
    region::Region reg4(q, h, unboundedVec1, x0, boundedVec1, {});
    region::Region reg5(q, h, unboundedVec1, x0, boundedVec2, {});
    region::Region reg6(q, h, unboundedVec2, x0, boundedVec0, {});
    region::Region reg7(q, h, unboundedVec2, x0, boundedVec1, {});
    region::Region reg8(q, h, unboundedVec2, x0, boundedVec2, {});


    std::vector startingRegions = { reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8 };

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


void testFischerFlat5()
{
    std::cout << "\n\nFischer Flat 5" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/fischerFlat/ff_05/";
    const std::string automatonFileName = "ff.txt";
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

    const int goal = locToIntMap.at("req1_req2_req3_cs4_cs5_id5");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", GT, 1);
    intVarOrClockConstr.emplace_back("x1", LT, 2);
    intVarOrClockConstr.emplace_back("x2", GT, 1);
    intVarOrClockConstr.emplace_back("x2", LT, 2);
    intVarOrClockConstr.emplace_back("x3", GT, 1);
    intVarOrClockConstr.emplace_back("x3", LT, 2);
    intVarOrClockConstr.emplace_back("x4", GT, 2);
    intVarOrClockConstr.emplace_back("x5", GT, 2);

    std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    // intero che rappresenta req1_req2_req3_cs4_cs5_id5: 2868

    std::cout << "\n\n";

    constexpr int q = 2868;
    const std::vector h = { 1, 1, 1, 2, 2 };

    boost::dynamic_bitset<> unbounded0(5);
    boost::dynamic_bitset<> unbounded1(5);
    unbounded0[0] = true;
    unbounded1[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec0;
    unboundedVec0.push_front(unbounded0);
    unboundedVec0.push_front(unbounded1);

    std::deque<boost::dynamic_bitset<>> unboundedVec1;
    unboundedVec1.push_front(unbounded1);
    unboundedVec1.push_front(unbounded0);

    boost::dynamic_bitset<> unbounded2(5);
    unbounded2[0] = true;
    unbounded2[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec2;
    unboundedVec2.push_front(unbounded2);


    // Ordered partition 0: [[2], [3], [4]]
    boost::dynamic_bitset<> bounded0(5);
    bounded0[2] = true;
    boost::dynamic_bitset<> bounded1(5);
    bounded1[3] = true;
    boost::dynamic_bitset<> bounded2(5);
    bounded2[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec0;
    boundedVec0.push_front(bounded2);
    boundedVec0.push_front(bounded1);
    boundedVec0.push_front(bounded0);


    // Ordered partition 1: [[2], [4], [3]]
    boost::dynamic_bitset<> bounded3(5);
    bounded3[2] = true;
    boost::dynamic_bitset<> bounded4(5);
    bounded4[4] = true;
    boost::dynamic_bitset<> bounded5(5);
    bounded5[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec1;
    boundedVec1.push_front(bounded5);
    boundedVec1.push_front(bounded4);
    boundedVec1.push_front(bounded3);


    // Ordered partition 2: [[3], [2], [4]]
    boost::dynamic_bitset<> bounded6(5);
    bounded6[3] = true;
    boost::dynamic_bitset<> bounded7(5);
    bounded7[2] = true;
    boost::dynamic_bitset<> bounded8(5);
    bounded8[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec2;
    boundedVec2.push_front(bounded8);
    boundedVec2.push_front(bounded7);
    boundedVec2.push_front(bounded6);


    // Ordered partition 3: [[3], [4], [2]]
    boost::dynamic_bitset<> bounded9(5);
    bounded9[3] = true;
    boost::dynamic_bitset<> bounded10(5);
    bounded10[4] = true;
    boost::dynamic_bitset<> bounded11(5);
    bounded11[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec3;
    boundedVec3.push_front(bounded11);
    boundedVec3.push_front(bounded10);
    boundedVec3.push_front(bounded9);


    // Ordered partition 4: [[4], [2], [3]]
    boost::dynamic_bitset<> bounded12(5);
    bounded12[4] = true;
    boost::dynamic_bitset<> bounded13(5);
    bounded13[2] = true;
    boost::dynamic_bitset<> bounded14(5);
    bounded14[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec4;
    boundedVec4.push_front(bounded14);
    boundedVec4.push_front(bounded13);
    boundedVec4.push_front(bounded12);


    // Ordered partition 5: [[4], [3], [2]]
    boost::dynamic_bitset<> bounded15(5);
    bounded15[4] = true;
    boost::dynamic_bitset<> bounded16(5);
    bounded16[3] = true;
    boost::dynamic_bitset<> bounded17(5);
    bounded17[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec5;
    boundedVec5.push_front(bounded17);
    boundedVec5.push_front(bounded16);
    boundedVec5.push_front(bounded15);


    // Ordered partition 6: [[2, 3], [4]]
    boost::dynamic_bitset<> bounded18(5);
    bounded18[2] = true;
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
    bounded21[2] = true;
    bounded21[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec7;
    boundedVec7.push_front(bounded21);
    boundedVec7.push_front(bounded20);


    // Ordered partition 8: [[3], [2, 4]]
    boost::dynamic_bitset<> bounded22(5);
    bounded22[3] = true;
    boost::dynamic_bitset<> bounded23(5);
    bounded23[2] = true;
    bounded23[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec8;
    boundedVec8.push_front(bounded23);
    boundedVec8.push_front(bounded22);


    // Ordered partition 9: [[2, 4], [3]]
    boost::dynamic_bitset<> bounded24(5);
    bounded24[2] = true;
    bounded24[4] = true;
    boost::dynamic_bitset<> bounded25(5);
    bounded25[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec9;
    boundedVec9.push_front(bounded25);
    boundedVec9.push_front(bounded24);


    // Ordered partition 10: [[2], [3, 4]]
    boost::dynamic_bitset<> bounded26(5);
    bounded26[2] = true;
    boost::dynamic_bitset<> bounded27(5);
    bounded27[3] = true;
    bounded27[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec10;
    boundedVec10.push_front(bounded27);
    boundedVec10.push_front(bounded26);


    // Ordered partition 11: [[3, 4], [2]]
    boost::dynamic_bitset<> bounded28(5);
    bounded28[3] = true;
    bounded28[4] = true;
    boost::dynamic_bitset<> bounded29(5);
    bounded29[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec11;
    boundedVec11.push_front(bounded29);
    boundedVec11.push_front(bounded28);


    // Ordered partition 12: [[2, 3, 4]]
    boost::dynamic_bitset<> bounded30(5);
    bounded30[2] = true;
    bounded30[3] = true;
    bounded30[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec12;
    boundedVec12.push_front(bounded30);

    boost::dynamic_bitset<> x0(5);

    region::Region reg0(q, h, unboundedVec0, x0, boundedVec0, {});
    region::Region reg1(q, h, unboundedVec0, x0, boundedVec1, {});
    region::Region reg2(q, h, unboundedVec0, x0, boundedVec2, {});
    region::Region reg3(q, h, unboundedVec0, x0, boundedVec3, {});
    region::Region reg4(q, h, unboundedVec0, x0, boundedVec4, {});
    region::Region reg5(q, h, unboundedVec0, x0, boundedVec5, {});
    region::Region reg6(q, h, unboundedVec0, x0, boundedVec6, {});
    region::Region reg7(q, h, unboundedVec0, x0, boundedVec7, {});
    region::Region reg8(q, h, unboundedVec0, x0, boundedVec8, {});
    region::Region reg9(q, h, unboundedVec0, x0, boundedVec9, {});
    region::Region reg10(q, h, unboundedVec0, x0, boundedVec10, {});
    region::Region reg11(q, h, unboundedVec0, x0, boundedVec11, {});

    region::Region reg12(q, h, unboundedVec1, x0, boundedVec0, {});
    region::Region reg13(q, h, unboundedVec1, x0, boundedVec1, {});
    region::Region reg14(q, h, unboundedVec1, x0, boundedVec2, {});
    region::Region reg15(q, h, unboundedVec1, x0, boundedVec3, {});
    region::Region reg16(q, h, unboundedVec1, x0, boundedVec4, {});
    region::Region reg17(q, h, unboundedVec1, x0, boundedVec5, {});
    region::Region reg18(q, h, unboundedVec1, x0, boundedVec6, {});
    region::Region reg19(q, h, unboundedVec1, x0, boundedVec7, {});
    region::Region reg20(q, h, unboundedVec1, x0, boundedVec8, {});
    region::Region reg21(q, h, unboundedVec1, x0, boundedVec9, {});
    region::Region reg22(q, h, unboundedVec1, x0, boundedVec10, {});
    region::Region reg23(q, h, unboundedVec1, x0, boundedVec11, {});

    region::Region reg24(q, h, unboundedVec2, x0, boundedVec0, {});
    region::Region reg25(q, h, unboundedVec2, x0, boundedVec1, {});
    region::Region reg26(q, h, unboundedVec2, x0, boundedVec2, {});
    region::Region reg27(q, h, unboundedVec2, x0, boundedVec3, {});
    region::Region reg28(q, h, unboundedVec2, x0, boundedVec4, {});
    region::Region reg29(q, h, unboundedVec2, x0, boundedVec5, {});
    region::Region reg30(q, h, unboundedVec2, x0, boundedVec6, {});
    region::Region reg31(q, h, unboundedVec2, x0, boundedVec7, {});
    region::Region reg32(q, h, unboundedVec2, x0, boundedVec8, {});
    region::Region reg33(q, h, unboundedVec2, x0, boundedVec9, {});
    region::Region reg34(q, h, unboundedVec2, x0, boundedVec10, {});
    region::Region reg35(q, h, unboundedVec2, x0, boundedVec11, {});

    std::vector startingRegions = {
        reg0, reg1, reg2, reg3, reg4, reg5,
        reg6, reg7, reg8, reg9, reg10, reg11,
        reg12, reg13, reg14, reg15, reg16, reg17,
        reg18, reg19, reg20, reg21, reg22, reg23,
        reg24, reg25, reg26, reg27, reg28, reg29,
        reg30, reg31, reg32, reg33, reg34, reg35
    };

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


void testFischerFlat6()
{
    std::cout << "\n\nFischer Flat 6" << std::endl;

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/fischerFlat/ff_06/";
    const std::string automatonFileName = "ff.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Redirect cout to file
    std::ofstream outFile(path + "rts_output.txt");
    std::streambuf *coutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(outFile.rdbuf());

    // std::cout << "\n\n\n";
    // std::cout << regionTransitionSystem.to_string() << std::endl;

    // Restore cout to terminal
    std::cout.rdbuf(coutBuffer);

    // const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    // const int goal = locToIntMap.at("req1_req2_req3_req4_cs5_cs6_id6");

    std::vector<timed_automaton::ast::clockConstraint> intVarOrClockConstr{};
    intVarOrClockConstr.emplace_back("x1", GT, 1);
    intVarOrClockConstr.emplace_back("x1", LT, 2);
    intVarOrClockConstr.emplace_back("x2", GT, 1);
    intVarOrClockConstr.emplace_back("x2", LT, 2);
    intVarOrClockConstr.emplace_back("x3", GT, 1);
    intVarOrClockConstr.emplace_back("x3", LT, 2);
    intVarOrClockConstr.emplace_back("x4", GT, 1);
    intVarOrClockConstr.emplace_back("x4", LT, 2);
    intVarOrClockConstr.emplace_back("x5", GT, 2);
    intVarOrClockConstr.emplace_back("x6", GT, 2);

    // It goes out of memory with forward exploration.
    // std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(intVarOrClockConstr, goal, DFS, false);

    // intero che rappresenta req1_req2_req3_req4_cs5_cs6_id6: 27734

    std::cout << "\n\n";

    constexpr int q = 27734;
    const std::vector h = { 1, 1, 1, 1, 2, 2 };

    boost::dynamic_bitset<> unbounded0(6);
    boost::dynamic_bitset<> unbounded1(6);
    unbounded0[0] = true;
    unbounded1[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec0;
    unboundedVec0.push_front(unbounded0);
    unboundedVec0.push_front(unbounded1);

    std::deque<boost::dynamic_bitset<>> unboundedVec1;
    unboundedVec1.push_front(unbounded1);
    unboundedVec1.push_front(unbounded0);

    boost::dynamic_bitset<> unbounded2(6);
    unbounded2[0] = true;
    unbounded2[1] = true;

    std::deque<boost::dynamic_bitset<>> unboundedVec2;
    unboundedVec2.push_front(unbounded2);

    // Total ordered partitions: 75
    // Elements: [2, 3, 4, 5]

    // Ordered partition 0: [[2], [3], [4], [5]]
    boost::dynamic_bitset<> bounded0(6);
    bounded0[2] = true;
    boost::dynamic_bitset<> bounded1(6);
    bounded1[3] = true;
    boost::dynamic_bitset<> bounded2(6);
    bounded2[4] = true;
    boost::dynamic_bitset<> bounded3(6);
    bounded3[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec0;
    boundedVec0.push_front(bounded3);
    boundedVec0.push_front(bounded2);
    boundedVec0.push_front(bounded1);
    boundedVec0.push_front(bounded0);


    // Ordered partition 1: [[2], [3], [5], [4]]
    boost::dynamic_bitset<> bounded4(6);
    bounded4[2] = true;
    boost::dynamic_bitset<> bounded5(6);
    bounded5[3] = true;
    boost::dynamic_bitset<> bounded6(6);
    bounded6[5] = true;
    boost::dynamic_bitset<> bounded7(6);
    bounded7[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec1;
    boundedVec1.push_front(bounded7);
    boundedVec1.push_front(bounded6);
    boundedVec1.push_front(bounded5);
    boundedVec1.push_front(bounded4);


    // Ordered partition 2: [[2], [4], [3], [5]]
    boost::dynamic_bitset<> bounded8(6);
    bounded8[2] = true;
    boost::dynamic_bitset<> bounded9(6);
    bounded9[4] = true;
    boost::dynamic_bitset<> bounded10(6);
    bounded10[3] = true;
    boost::dynamic_bitset<> bounded11(6);
    bounded11[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec2;
    boundedVec2.push_front(bounded11);
    boundedVec2.push_front(bounded10);
    boundedVec2.push_front(bounded9);
    boundedVec2.push_front(bounded8);


    // Ordered partition 3: [[2], [4], [5], [3]]
    boost::dynamic_bitset<> bounded12(6);
    bounded12[2] = true;
    boost::dynamic_bitset<> bounded13(6);
    bounded13[4] = true;
    boost::dynamic_bitset<> bounded14(6);
    bounded14[5] = true;
    boost::dynamic_bitset<> bounded15(6);
    bounded15[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec3;
    boundedVec3.push_front(bounded15);
    boundedVec3.push_front(bounded14);
    boundedVec3.push_front(bounded13);
    boundedVec3.push_front(bounded12);


    // Ordered partition 4: [[2], [5], [3], [4]]
    boost::dynamic_bitset<> bounded16(6);
    bounded16[2] = true;
    boost::dynamic_bitset<> bounded17(6);
    bounded17[5] = true;
    boost::dynamic_bitset<> bounded18(6);
    bounded18[3] = true;
    boost::dynamic_bitset<> bounded19(6);
    bounded19[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec4;
    boundedVec4.push_front(bounded19);
    boundedVec4.push_front(bounded18);
    boundedVec4.push_front(bounded17);
    boundedVec4.push_front(bounded16);


    // Ordered partition 5: [[2], [5], [4], [3]]
    boost::dynamic_bitset<> bounded20(6);
    bounded20[2] = true;
    boost::dynamic_bitset<> bounded21(6);
    bounded21[5] = true;
    boost::dynamic_bitset<> bounded22(6);
    bounded22[4] = true;
    boost::dynamic_bitset<> bounded23(6);
    bounded23[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec5;
    boundedVec5.push_front(bounded23);
    boundedVec5.push_front(bounded22);
    boundedVec5.push_front(bounded21);
    boundedVec5.push_front(bounded20);


    // Ordered partition 6: [[3], [2], [4], [5]]
    boost::dynamic_bitset<> bounded24(6);
    bounded24[3] = true;
    boost::dynamic_bitset<> bounded25(6);
    bounded25[2] = true;
    boost::dynamic_bitset<> bounded26(6);
    bounded26[4] = true;
    boost::dynamic_bitset<> bounded27(6);
    bounded27[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec6;
    boundedVec6.push_front(bounded27);
    boundedVec6.push_front(bounded26);
    boundedVec6.push_front(bounded25);
    boundedVec6.push_front(bounded24);


    // Ordered partition 7: [[3], [2], [5], [4]]
    boost::dynamic_bitset<> bounded28(6);
    bounded28[3] = true;
    boost::dynamic_bitset<> bounded29(6);
    bounded29[2] = true;
    boost::dynamic_bitset<> bounded30(6);
    bounded30[5] = true;
    boost::dynamic_bitset<> bounded31(6);
    bounded31[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec7;
    boundedVec7.push_front(bounded31);
    boundedVec7.push_front(bounded30);
    boundedVec7.push_front(bounded29);
    boundedVec7.push_front(bounded28);


    // Ordered partition 8: [[3], [4], [2], [5]]
    boost::dynamic_bitset<> bounded32(6);
    bounded32[3] = true;
    boost::dynamic_bitset<> bounded33(6);
    bounded33[4] = true;
    boost::dynamic_bitset<> bounded34(6);
    bounded34[2] = true;
    boost::dynamic_bitset<> bounded35(6);
    bounded35[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec8;
    boundedVec8.push_front(bounded35);
    boundedVec8.push_front(bounded34);
    boundedVec8.push_front(bounded33);
    boundedVec8.push_front(bounded32);


    // Ordered partition 9: [[3], [4], [5], [2]]
    boost::dynamic_bitset<> bounded36(6);
    bounded36[3] = true;
    boost::dynamic_bitset<> bounded37(6);
    bounded37[4] = true;
    boost::dynamic_bitset<> bounded38(6);
    bounded38[5] = true;
    boost::dynamic_bitset<> bounded39(6);
    bounded39[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec9;
    boundedVec9.push_front(bounded39);
    boundedVec9.push_front(bounded38);
    boundedVec9.push_front(bounded37);
    boundedVec9.push_front(bounded36);


    // Ordered partition 10: [[3], [5], [2], [4]]
    boost::dynamic_bitset<> bounded40(6);
    bounded40[3] = true;
    boost::dynamic_bitset<> bounded41(6);
    bounded41[5] = true;
    boost::dynamic_bitset<> bounded42(6);
    bounded42[2] = true;
    boost::dynamic_bitset<> bounded43(6);
    bounded43[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec10;
    boundedVec10.push_front(bounded43);
    boundedVec10.push_front(bounded42);
    boundedVec10.push_front(bounded41);
    boundedVec10.push_front(bounded40);


    // Ordered partition 11: [[3], [5], [4], [2]]
    boost::dynamic_bitset<> bounded44(6);
    bounded44[3] = true;
    boost::dynamic_bitset<> bounded45(6);
    bounded45[5] = true;
    boost::dynamic_bitset<> bounded46(6);
    bounded46[4] = true;
    boost::dynamic_bitset<> bounded47(6);
    bounded47[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec11;
    boundedVec11.push_front(bounded47);
    boundedVec11.push_front(bounded46);
    boundedVec11.push_front(bounded45);
    boundedVec11.push_front(bounded44);


    // Ordered partition 12: [[4], [2], [3], [5]]
    boost::dynamic_bitset<> bounded48(6);
    bounded48[4] = true;
    boost::dynamic_bitset<> bounded49(6);
    bounded49[2] = true;
    boost::dynamic_bitset<> bounded50(6);
    bounded50[3] = true;
    boost::dynamic_bitset<> bounded51(6);
    bounded51[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec12;
    boundedVec12.push_front(bounded51);
    boundedVec12.push_front(bounded50);
    boundedVec12.push_front(bounded49);
    boundedVec12.push_front(bounded48);


    // Ordered partition 13: [[4], [2], [5], [3]]
    boost::dynamic_bitset<> bounded52(6);
    bounded52[4] = true;
    boost::dynamic_bitset<> bounded53(6);
    bounded53[2] = true;
    boost::dynamic_bitset<> bounded54(6);
    bounded54[5] = true;
    boost::dynamic_bitset<> bounded55(6);
    bounded55[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec13;
    boundedVec13.push_front(bounded55);
    boundedVec13.push_front(bounded54);
    boundedVec13.push_front(bounded53);
    boundedVec13.push_front(bounded52);


    // Ordered partition 14: [[4], [3], [2], [5]]
    boost::dynamic_bitset<> bounded56(6);
    bounded56[4] = true;
    boost::dynamic_bitset<> bounded57(6);
    bounded57[3] = true;
    boost::dynamic_bitset<> bounded58(6);
    bounded58[2] = true;
    boost::dynamic_bitset<> bounded59(6);
    bounded59[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec14;
    boundedVec14.push_front(bounded59);
    boundedVec14.push_front(bounded58);
    boundedVec14.push_front(bounded57);
    boundedVec14.push_front(bounded56);


    // Ordered partition 15: [[4], [3], [5], [2]]
    boost::dynamic_bitset<> bounded60(6);
    bounded60[4] = true;
    boost::dynamic_bitset<> bounded61(6);
    bounded61[3] = true;
    boost::dynamic_bitset<> bounded62(6);
    bounded62[5] = true;
    boost::dynamic_bitset<> bounded63(6);
    bounded63[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec15;
    boundedVec15.push_front(bounded63);
    boundedVec15.push_front(bounded62);
    boundedVec15.push_front(bounded61);
    boundedVec15.push_front(bounded60);


    // Ordered partition 16: [[4], [5], [2], [3]]
    boost::dynamic_bitset<> bounded64(6);
    bounded64[4] = true;
    boost::dynamic_bitset<> bounded65(6);
    bounded65[5] = true;
    boost::dynamic_bitset<> bounded66(6);
    bounded66[2] = true;
    boost::dynamic_bitset<> bounded67(6);
    bounded67[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec16;
    boundedVec16.push_front(bounded67);
    boundedVec16.push_front(bounded66);
    boundedVec16.push_front(bounded65);
    boundedVec16.push_front(bounded64);


    // Ordered partition 17: [[4], [5], [3], [2]]
    boost::dynamic_bitset<> bounded68(6);
    bounded68[4] = true;
    boost::dynamic_bitset<> bounded69(6);
    bounded69[5] = true;
    boost::dynamic_bitset<> bounded70(6);
    bounded70[3] = true;
    boost::dynamic_bitset<> bounded71(6);
    bounded71[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec17;
    boundedVec17.push_front(bounded71);
    boundedVec17.push_front(bounded70);
    boundedVec17.push_front(bounded69);
    boundedVec17.push_front(bounded68);


    // Ordered partition 18: [[5], [2], [3], [4]]
    boost::dynamic_bitset<> bounded72(6);
    bounded72[5] = true;
    boost::dynamic_bitset<> bounded73(6);
    bounded73[2] = true;
    boost::dynamic_bitset<> bounded74(6);
    bounded74[3] = true;
    boost::dynamic_bitset<> bounded75(6);
    bounded75[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec18;
    boundedVec18.push_front(bounded75);
    boundedVec18.push_front(bounded74);
    boundedVec18.push_front(bounded73);
    boundedVec18.push_front(bounded72);


    // Ordered partition 19: [[5], [2], [4], [3]]
    boost::dynamic_bitset<> bounded76(6);
    bounded76[5] = true;
    boost::dynamic_bitset<> bounded77(6);
    bounded77[2] = true;
    boost::dynamic_bitset<> bounded78(6);
    bounded78[4] = true;
    boost::dynamic_bitset<> bounded79(6);
    bounded79[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec19;
    boundedVec19.push_front(bounded79);
    boundedVec19.push_front(bounded78);
    boundedVec19.push_front(bounded77);
    boundedVec19.push_front(bounded76);


    // Ordered partition 20: [[5], [3], [2], [4]]
    boost::dynamic_bitset<> bounded80(6);
    bounded80[5] = true;
    boost::dynamic_bitset<> bounded81(6);
    bounded81[3] = true;
    boost::dynamic_bitset<> bounded82(6);
    bounded82[2] = true;
    boost::dynamic_bitset<> bounded83(6);
    bounded83[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec20;
    boundedVec20.push_front(bounded83);
    boundedVec20.push_front(bounded82);
    boundedVec20.push_front(bounded81);
    boundedVec20.push_front(bounded80);


    // Ordered partition 21: [[5], [3], [4], [2]]
    boost::dynamic_bitset<> bounded84(6);
    bounded84[5] = true;
    boost::dynamic_bitset<> bounded85(6);
    bounded85[3] = true;
    boost::dynamic_bitset<> bounded86(6);
    bounded86[4] = true;
    boost::dynamic_bitset<> bounded87(6);
    bounded87[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec21;
    boundedVec21.push_front(bounded87);
    boundedVec21.push_front(bounded86);
    boundedVec21.push_front(bounded85);
    boundedVec21.push_front(bounded84);


    // Ordered partition 22: [[5], [4], [2], [3]]
    boost::dynamic_bitset<> bounded88(6);
    bounded88[5] = true;
    boost::dynamic_bitset<> bounded89(6);
    bounded89[4] = true;
    boost::dynamic_bitset<> bounded90(6);
    bounded90[2] = true;
    boost::dynamic_bitset<> bounded91(6);
    bounded91[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec22;
    boundedVec22.push_front(bounded91);
    boundedVec22.push_front(bounded90);
    boundedVec22.push_front(bounded89);
    boundedVec22.push_front(bounded88);


    // Ordered partition 23: [[5], [4], [3], [2]]
    boost::dynamic_bitset<> bounded92(6);
    bounded92[5] = true;
    boost::dynamic_bitset<> bounded93(6);
    bounded93[4] = true;
    boost::dynamic_bitset<> bounded94(6);
    bounded94[3] = true;
    boost::dynamic_bitset<> bounded95(6);
    bounded95[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec23;
    boundedVec23.push_front(bounded95);
    boundedVec23.push_front(bounded94);
    boundedVec23.push_front(bounded93);
    boundedVec23.push_front(bounded92);


    // Ordered partition 24: [[2, 3], [4], [5]]
    boost::dynamic_bitset<> bounded96(6);
    bounded96[2] = true;
    bounded96[3] = true;
    boost::dynamic_bitset<> bounded97(6);
    bounded97[4] = true;
    boost::dynamic_bitset<> bounded98(6);
    bounded98[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec24;
    boundedVec24.push_front(bounded98);
    boundedVec24.push_front(bounded97);
    boundedVec24.push_front(bounded96);


    // Ordered partition 25: [[2, 3], [5], [4]]
    boost::dynamic_bitset<> bounded99(6);
    bounded99[2] = true;
    bounded99[3] = true;
    boost::dynamic_bitset<> bounded100(6);
    bounded100[5] = true;
    boost::dynamic_bitset<> bounded101(6);
    bounded101[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec25;
    boundedVec25.push_front(bounded101);
    boundedVec25.push_front(bounded100);
    boundedVec25.push_front(bounded99);


    // Ordered partition 26: [[4], [2, 3], [5]]
    boost::dynamic_bitset<> bounded102(6);
    bounded102[4] = true;
    boost::dynamic_bitset<> bounded103(6);
    bounded103[2] = true;
    bounded103[3] = true;
    boost::dynamic_bitset<> bounded104(6);
    bounded104[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec26;
    boundedVec26.push_front(bounded104);
    boundedVec26.push_front(bounded103);
    boundedVec26.push_front(bounded102);


    // Ordered partition 27: [[4], [5], [2, 3]]
    boost::dynamic_bitset<> bounded105(6);
    bounded105[4] = true;
    boost::dynamic_bitset<> bounded106(6);
    bounded106[5] = true;
    boost::dynamic_bitset<> bounded107(6);
    bounded107[2] = true;
    bounded107[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec27;
    boundedVec27.push_front(bounded107);
    boundedVec27.push_front(bounded106);
    boundedVec27.push_front(bounded105);


    // Ordered partition 28: [[5], [2, 3], [4]]
    boost::dynamic_bitset<> bounded108(6);
    bounded108[5] = true;
    boost::dynamic_bitset<> bounded109(6);
    bounded109[2] = true;
    bounded109[3] = true;
    boost::dynamic_bitset<> bounded110(6);
    bounded110[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec28;
    boundedVec28.push_front(bounded110);
    boundedVec28.push_front(bounded109);
    boundedVec28.push_front(bounded108);


    // Ordered partition 29: [[5], [4], [2, 3]]
    boost::dynamic_bitset<> bounded111(6);
    bounded111[5] = true;
    boost::dynamic_bitset<> bounded112(6);
    bounded112[4] = true;
    boost::dynamic_bitset<> bounded113(6);
    bounded113[2] = true;
    bounded113[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec29;
    boundedVec29.push_front(bounded113);
    boundedVec29.push_front(bounded112);
    boundedVec29.push_front(bounded111);


    // Ordered partition 30: [[3], [2, 4], [5]]
    boost::dynamic_bitset<> bounded114(6);
    bounded114[3] = true;
    boost::dynamic_bitset<> bounded115(6);
    bounded115[2] = true;
    bounded115[4] = true;
    boost::dynamic_bitset<> bounded116(6);
    bounded116[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec30;
    boundedVec30.push_front(bounded116);
    boundedVec30.push_front(bounded115);
    boundedVec30.push_front(bounded114);


    // Ordered partition 31: [[3], [5], [2, 4]]
    boost::dynamic_bitset<> bounded117(6);
    bounded117[3] = true;
    boost::dynamic_bitset<> bounded118(6);
    bounded118[5] = true;
    boost::dynamic_bitset<> bounded119(6);
    bounded119[2] = true;
    bounded119[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec31;
    boundedVec31.push_front(bounded119);
    boundedVec31.push_front(bounded118);
    boundedVec31.push_front(bounded117);


    // Ordered partition 32: [[2, 4], [3], [5]]
    boost::dynamic_bitset<> bounded120(6);
    bounded120[2] = true;
    bounded120[4] = true;
    boost::dynamic_bitset<> bounded121(6);
    bounded121[3] = true;
    boost::dynamic_bitset<> bounded122(6);
    bounded122[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec32;
    boundedVec32.push_front(bounded122);
    boundedVec32.push_front(bounded121);
    boundedVec32.push_front(bounded120);


    // Ordered partition 33: [[2, 4], [5], [3]]
    boost::dynamic_bitset<> bounded123(6);
    bounded123[2] = true;
    bounded123[4] = true;
    boost::dynamic_bitset<> bounded124(6);
    bounded124[5] = true;
    boost::dynamic_bitset<> bounded125(6);
    bounded125[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec33;
    boundedVec33.push_front(bounded125);
    boundedVec33.push_front(bounded124);
    boundedVec33.push_front(bounded123);


    // Ordered partition 34: [[5], [3], [2, 4]]
    boost::dynamic_bitset<> bounded126(6);
    bounded126[5] = true;
    boost::dynamic_bitset<> bounded127(6);
    bounded127[3] = true;
    boost::dynamic_bitset<> bounded128(6);
    bounded128[2] = true;
    bounded128[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec34;
    boundedVec34.push_front(bounded128);
    boundedVec34.push_front(bounded127);
    boundedVec34.push_front(bounded126);


    // Ordered partition 35: [[5], [2, 4], [3]]
    boost::dynamic_bitset<> bounded129(6);
    bounded129[5] = true;
    boost::dynamic_bitset<> bounded130(6);
    bounded130[2] = true;
    bounded130[4] = true;
    boost::dynamic_bitset<> bounded131(6);
    bounded131[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec35;
    boundedVec35.push_front(bounded131);
    boundedVec35.push_front(bounded130);
    boundedVec35.push_front(bounded129);


    // Ordered partition 36: [[3], [4], [2, 5]]
    boost::dynamic_bitset<> bounded132(6);
    bounded132[3] = true;
    boost::dynamic_bitset<> bounded133(6);
    bounded133[4] = true;
    boost::dynamic_bitset<> bounded134(6);
    bounded134[2] = true;
    bounded134[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec36;
    boundedVec36.push_front(bounded134);
    boundedVec36.push_front(bounded133);
    boundedVec36.push_front(bounded132);


    // Ordered partition 37: [[3], [2, 5], [4]]
    boost::dynamic_bitset<> bounded135(6);
    bounded135[3] = true;
    boost::dynamic_bitset<> bounded136(6);
    bounded136[2] = true;
    bounded136[5] = true;
    boost::dynamic_bitset<> bounded137(6);
    bounded137[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec37;
    boundedVec37.push_front(bounded137);
    boundedVec37.push_front(bounded136);
    boundedVec37.push_front(bounded135);


    // Ordered partition 38: [[4], [3], [2, 5]]
    boost::dynamic_bitset<> bounded138(6);
    bounded138[4] = true;
    boost::dynamic_bitset<> bounded139(6);
    bounded139[3] = true;
    boost::dynamic_bitset<> bounded140(6);
    bounded140[2] = true;
    bounded140[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec38;
    boundedVec38.push_front(bounded140);
    boundedVec38.push_front(bounded139);
    boundedVec38.push_front(bounded138);


    // Ordered partition 39: [[4], [2, 5], [3]]
    boost::dynamic_bitset<> bounded141(6);
    bounded141[4] = true;
    boost::dynamic_bitset<> bounded142(6);
    bounded142[2] = true;
    bounded142[5] = true;
    boost::dynamic_bitset<> bounded143(6);
    bounded143[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec39;
    boundedVec39.push_front(bounded143);
    boundedVec39.push_front(bounded142);
    boundedVec39.push_front(bounded141);


    // Ordered partition 40: [[2, 5], [3], [4]]
    boost::dynamic_bitset<> bounded144(6);
    bounded144[2] = true;
    bounded144[5] = true;
    boost::dynamic_bitset<> bounded145(6);
    bounded145[3] = true;
    boost::dynamic_bitset<> bounded146(6);
    bounded146[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec40;
    boundedVec40.push_front(bounded146);
    boundedVec40.push_front(bounded145);
    boundedVec40.push_front(bounded144);


    // Ordered partition 41: [[2, 5], [4], [3]]
    boost::dynamic_bitset<> bounded147(6);
    bounded147[2] = true;
    bounded147[5] = true;
    boost::dynamic_bitset<> bounded148(6);
    bounded148[4] = true;
    boost::dynamic_bitset<> bounded149(6);
    bounded149[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec41;
    boundedVec41.push_front(bounded149);
    boundedVec41.push_front(bounded148);
    boundedVec41.push_front(bounded147);


    // Ordered partition 42: [[2], [3, 4], [5]]
    boost::dynamic_bitset<> bounded150(6);
    bounded150[2] = true;
    boost::dynamic_bitset<> bounded151(6);
    bounded151[3] = true;
    bounded151[4] = true;
    boost::dynamic_bitset<> bounded152(6);
    bounded152[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec42;
    boundedVec42.push_front(bounded152);
    boundedVec42.push_front(bounded151);
    boundedVec42.push_front(bounded150);


    // Ordered partition 43: [[2], [5], [3, 4]]
    boost::dynamic_bitset<> bounded153(6);
    bounded153[2] = true;
    boost::dynamic_bitset<> bounded154(6);
    bounded154[5] = true;
    boost::dynamic_bitset<> bounded155(6);
    bounded155[3] = true;
    bounded155[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec43;
    boundedVec43.push_front(bounded155);
    boundedVec43.push_front(bounded154);
    boundedVec43.push_front(bounded153);


    // Ordered partition 44: [[3, 4], [2], [5]]
    boost::dynamic_bitset<> bounded156(6);
    bounded156[3] = true;
    bounded156[4] = true;
    boost::dynamic_bitset<> bounded157(6);
    bounded157[2] = true;
    boost::dynamic_bitset<> bounded158(6);
    bounded158[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec44;
    boundedVec44.push_front(bounded158);
    boundedVec44.push_front(bounded157);
    boundedVec44.push_front(bounded156);


    // Ordered partition 45: [[3, 4], [5], [2]]
    boost::dynamic_bitset<> bounded159(6);
    bounded159[3] = true;
    bounded159[4] = true;
    boost::dynamic_bitset<> bounded160(6);
    bounded160[5] = true;
    boost::dynamic_bitset<> bounded161(6);
    bounded161[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec45;
    boundedVec45.push_front(bounded161);
    boundedVec45.push_front(bounded160);
    boundedVec45.push_front(bounded159);


    // Ordered partition 46: [[5], [2], [3, 4]]
    boost::dynamic_bitset<> bounded162(6);
    bounded162[5] = true;
    boost::dynamic_bitset<> bounded163(6);
    bounded163[2] = true;
    boost::dynamic_bitset<> bounded164(6);
    bounded164[3] = true;
    bounded164[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec46;
    boundedVec46.push_front(bounded164);
    boundedVec46.push_front(bounded163);
    boundedVec46.push_front(bounded162);


    // Ordered partition 47: [[5], [3, 4], [2]]
    boost::dynamic_bitset<> bounded165(6);
    bounded165[5] = true;
    boost::dynamic_bitset<> bounded166(6);
    bounded166[3] = true;
    bounded166[4] = true;
    boost::dynamic_bitset<> bounded167(6);
    bounded167[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec47;
    boundedVec47.push_front(bounded167);
    boundedVec47.push_front(bounded166);
    boundedVec47.push_front(bounded165);


    // Ordered partition 48: [[2, 3, 4], [5]]
    boost::dynamic_bitset<> bounded168(6);
    bounded168[2] = true;
    bounded168[3] = true;
    bounded168[4] = true;
    boost::dynamic_bitset<> bounded169(6);
    bounded169[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec48;
    boundedVec48.push_front(bounded169);
    boundedVec48.push_front(bounded168);


    // Ordered partition 49: [[5], [2, 3, 4]]
    boost::dynamic_bitset<> bounded170(6);
    bounded170[5] = true;
    boost::dynamic_bitset<> bounded171(6);
    bounded171[2] = true;
    bounded171[3] = true;
    bounded171[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec49;
    boundedVec49.push_front(bounded171);
    boundedVec49.push_front(bounded170);


    // Ordered partition 50: [[3, 4], [2, 5]]
    boost::dynamic_bitset<> bounded172(6);
    bounded172[3] = true;
    bounded172[4] = true;
    boost::dynamic_bitset<> bounded173(6);
    bounded173[2] = true;
    bounded173[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec50;
    boundedVec50.push_front(bounded173);
    boundedVec50.push_front(bounded172);


    // Ordered partition 51: [[2, 5], [3, 4]]
    boost::dynamic_bitset<> bounded174(6);
    bounded174[2] = true;
    bounded174[5] = true;
    boost::dynamic_bitset<> bounded175(6);
    bounded175[3] = true;
    bounded175[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec51;
    boundedVec51.push_front(bounded175);
    boundedVec51.push_front(bounded174);


    // Ordered partition 52: [[2], [4], [3, 5]]
    boost::dynamic_bitset<> bounded176(6);
    bounded176[2] = true;
    boost::dynamic_bitset<> bounded177(6);
    bounded177[4] = true;
    boost::dynamic_bitset<> bounded178(6);
    bounded178[3] = true;
    bounded178[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec52;
    boundedVec52.push_front(bounded178);
    boundedVec52.push_front(bounded177);
    boundedVec52.push_front(bounded176);


    // Ordered partition 53: [[2], [3, 5], [4]]
    boost::dynamic_bitset<> bounded179(6);
    bounded179[2] = true;
    boost::dynamic_bitset<> bounded180(6);
    bounded180[3] = true;
    bounded180[5] = true;
    boost::dynamic_bitset<> bounded181(6);
    bounded181[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec53;
    boundedVec53.push_front(bounded181);
    boundedVec53.push_front(bounded180);
    boundedVec53.push_front(bounded179);


    // Ordered partition 54: [[4], [2], [3, 5]]
    boost::dynamic_bitset<> bounded182(6);
    bounded182[4] = true;
    boost::dynamic_bitset<> bounded183(6);
    bounded183[2] = true;
    boost::dynamic_bitset<> bounded184(6);
    bounded184[3] = true;
    bounded184[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec54;
    boundedVec54.push_front(bounded184);
    boundedVec54.push_front(bounded183);
    boundedVec54.push_front(bounded182);


    // Ordered partition 55: [[4], [3, 5], [2]]
    boost::dynamic_bitset<> bounded185(6);
    bounded185[4] = true;
    boost::dynamic_bitset<> bounded186(6);
    bounded186[3] = true;
    bounded186[5] = true;
    boost::dynamic_bitset<> bounded187(6);
    bounded187[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec55;
    boundedVec55.push_front(bounded187);
    boundedVec55.push_front(bounded186);
    boundedVec55.push_front(bounded185);


    // Ordered partition 56: [[3, 5], [2], [4]]
    boost::dynamic_bitset<> bounded188(6);
    bounded188[3] = true;
    bounded188[5] = true;
    boost::dynamic_bitset<> bounded189(6);
    bounded189[2] = true;
    boost::dynamic_bitset<> bounded190(6);
    bounded190[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec56;
    boundedVec56.push_front(bounded190);
    boundedVec56.push_front(bounded189);
    boundedVec56.push_front(bounded188);


    // Ordered partition 57: [[3, 5], [4], [2]]
    boost::dynamic_bitset<> bounded191(6);
    bounded191[3] = true;
    bounded191[5] = true;
    boost::dynamic_bitset<> bounded192(6);
    bounded192[4] = true;
    boost::dynamic_bitset<> bounded193(6);
    bounded193[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec57;
    boundedVec57.push_front(bounded193);
    boundedVec57.push_front(bounded192);
    boundedVec57.push_front(bounded191);


    // Ordered partition 58: [[2, 4], [3, 5]]
    boost::dynamic_bitset<> bounded194(6);
    bounded194[2] = true;
    bounded194[4] = true;
    boost::dynamic_bitset<> bounded195(6);
    bounded195[3] = true;
    bounded195[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec58;
    boundedVec58.push_front(bounded195);
    boundedVec58.push_front(bounded194);


    // Ordered partition 59: [[3, 5], [2, 4]]
    boost::dynamic_bitset<> bounded196(6);
    bounded196[3] = true;
    bounded196[5] = true;
    boost::dynamic_bitset<> bounded197(6);
    bounded197[2] = true;
    bounded197[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec59;
    boundedVec59.push_front(bounded197);
    boundedVec59.push_front(bounded196);


    // Ordered partition 60: [[4], [2, 3, 5]]
    boost::dynamic_bitset<> bounded198(6);
    bounded198[4] = true;
    boost::dynamic_bitset<> bounded199(6);
    bounded199[2] = true;
    bounded199[3] = true;
    bounded199[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec60;
    boundedVec60.push_front(bounded199);
    boundedVec60.push_front(bounded198);


    // Ordered partition 61: [[2, 3, 5], [4]]
    boost::dynamic_bitset<> bounded200(6);
    bounded200[2] = true;
    bounded200[3] = true;
    bounded200[5] = true;
    boost::dynamic_bitset<> bounded201(6);
    bounded201[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec61;
    boundedVec61.push_front(bounded201);
    boundedVec61.push_front(bounded200);


    // Ordered partition 62: [[2], [3], [4, 5]]
    boost::dynamic_bitset<> bounded202(6);
    bounded202[2] = true;
    boost::dynamic_bitset<> bounded203(6);
    bounded203[3] = true;
    boost::dynamic_bitset<> bounded204(6);
    bounded204[4] = true;
    bounded204[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec62;
    boundedVec62.push_front(bounded204);
    boundedVec62.push_front(bounded203);
    boundedVec62.push_front(bounded202);


    // Ordered partition 63: [[2], [4, 5], [3]]
    boost::dynamic_bitset<> bounded205(6);
    bounded205[2] = true;
    boost::dynamic_bitset<> bounded206(6);
    bounded206[4] = true;
    bounded206[5] = true;
    boost::dynamic_bitset<> bounded207(6);
    bounded207[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec63;
    boundedVec63.push_front(bounded207);
    boundedVec63.push_front(bounded206);
    boundedVec63.push_front(bounded205);


    // Ordered partition 64: [[3], [2], [4, 5]]
    boost::dynamic_bitset<> bounded208(6);
    bounded208[3] = true;
    boost::dynamic_bitset<> bounded209(6);
    bounded209[2] = true;
    boost::dynamic_bitset<> bounded210(6);
    bounded210[4] = true;
    bounded210[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec64;
    boundedVec64.push_front(bounded210);
    boundedVec64.push_front(bounded209);
    boundedVec64.push_front(bounded208);


    // Ordered partition 65: [[3], [4, 5], [2]]
    boost::dynamic_bitset<> bounded211(6);
    bounded211[3] = true;
    boost::dynamic_bitset<> bounded212(6);
    bounded212[4] = true;
    bounded212[5] = true;
    boost::dynamic_bitset<> bounded213(6);
    bounded213[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec65;
    boundedVec65.push_front(bounded213);
    boundedVec65.push_front(bounded212);
    boundedVec65.push_front(bounded211);


    // Ordered partition 66: [[4, 5], [2], [3]]
    boost::dynamic_bitset<> bounded214(6);
    bounded214[4] = true;
    bounded214[5] = true;
    boost::dynamic_bitset<> bounded215(6);
    bounded215[2] = true;
    boost::dynamic_bitset<> bounded216(6);
    bounded216[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec66;
    boundedVec66.push_front(bounded216);
    boundedVec66.push_front(bounded215);
    boundedVec66.push_front(bounded214);


    // Ordered partition 67: [[4, 5], [3], [2]]
    boost::dynamic_bitset<> bounded217(6);
    bounded217[4] = true;
    bounded217[5] = true;
    boost::dynamic_bitset<> bounded218(6);
    bounded218[3] = true;
    boost::dynamic_bitset<> bounded219(6);
    bounded219[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec67;
    boundedVec67.push_front(bounded219);
    boundedVec67.push_front(bounded218);
    boundedVec67.push_front(bounded217);


    // Ordered partition 68: [[2, 3], [4, 5]]
    boost::dynamic_bitset<> bounded220(6);
    bounded220[2] = true;
    bounded220[3] = true;
    boost::dynamic_bitset<> bounded221(6);
    bounded221[4] = true;
    bounded221[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec68;
    boundedVec68.push_front(bounded221);
    boundedVec68.push_front(bounded220);


    // Ordered partition 69: [[4, 5], [2, 3]]
    boost::dynamic_bitset<> bounded222(6);
    bounded222[4] = true;
    bounded222[5] = true;
    boost::dynamic_bitset<> bounded223(6);
    bounded223[2] = true;
    bounded223[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec69;
    boundedVec69.push_front(bounded223);
    boundedVec69.push_front(bounded222);


    // Ordered partition 70: [[3], [2, 4, 5]]
    boost::dynamic_bitset<> bounded224(6);
    bounded224[3] = true;
    boost::dynamic_bitset<> bounded225(6);
    bounded225[2] = true;
    bounded225[4] = true;
    bounded225[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec70;
    boundedVec70.push_front(bounded225);
    boundedVec70.push_front(bounded224);


    // Ordered partition 71: [[2, 4, 5], [3]]
    boost::dynamic_bitset<> bounded226(6);
    bounded226[2] = true;
    bounded226[4] = true;
    bounded226[5] = true;
    boost::dynamic_bitset<> bounded227(6);
    bounded227[3] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec71;
    boundedVec71.push_front(bounded227);
    boundedVec71.push_front(bounded226);


    // Ordered partition 72: [[2], [3, 4, 5]]
    boost::dynamic_bitset<> bounded228(6);
    bounded228[2] = true;
    boost::dynamic_bitset<> bounded229(6);
    bounded229[3] = true;
    bounded229[4] = true;
    bounded229[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec72;
    boundedVec72.push_front(bounded229);
    boundedVec72.push_front(bounded228);


    // Ordered partition 73: [[3, 4, 5], [2]]
    boost::dynamic_bitset<> bounded230(6);
    bounded230[3] = true;
    bounded230[4] = true;
    bounded230[5] = true;
    boost::dynamic_bitset<> bounded231(6);
    bounded231[2] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec73;
    boundedVec73.push_front(bounded231);
    boundedVec73.push_front(bounded230);


    // Ordered partition 74: [[2, 3, 4, 5]]
    boost::dynamic_bitset<> bounded232(6);
    bounded232[2] = true;
    bounded232[3] = true;
    bounded232[4] = true;
    bounded232[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec74;
    boundedVec74.push_front(bounded232);


    boost::dynamic_bitset<> x0(6);

    region::Region reg0(q, h, unboundedVec0, x0, boundedVec0, {});
    region::Region reg1(q, h, unboundedVec0, x0, boundedVec1, {});
    region::Region reg2(q, h, unboundedVec0, x0, boundedVec2, {});
    region::Region reg3(q, h, unboundedVec0, x0, boundedVec3, {});
    region::Region reg4(q, h, unboundedVec0, x0, boundedVec4, {});
    region::Region reg5(q, h, unboundedVec0, x0, boundedVec5, {});
    region::Region reg6(q, h, unboundedVec0, x0, boundedVec6, {});
    region::Region reg7(q, h, unboundedVec0, x0, boundedVec7, {});
    region::Region reg8(q, h, unboundedVec0, x0, boundedVec8, {});
    region::Region reg9(q, h, unboundedVec0, x0, boundedVec9, {});
    region::Region reg10(q, h, unboundedVec0, x0, boundedVec10, {});
    region::Region reg11(q, h, unboundedVec0, x0, boundedVec11, {});
    region::Region reg12(q, h, unboundedVec0, x0, boundedVec12, {});
    region::Region reg13(q, h, unboundedVec0, x0, boundedVec13, {});
    region::Region reg14(q, h, unboundedVec0, x0, boundedVec14, {});
    region::Region reg15(q, h, unboundedVec0, x0, boundedVec15, {});
    region::Region reg16(q, h, unboundedVec0, x0, boundedVec16, {});
    region::Region reg17(q, h, unboundedVec0, x0, boundedVec17, {});
    region::Region reg18(q, h, unboundedVec0, x0, boundedVec18, {});
    region::Region reg19(q, h, unboundedVec0, x0, boundedVec19, {});
    region::Region reg20(q, h, unboundedVec0, x0, boundedVec20, {});
    region::Region reg21(q, h, unboundedVec0, x0, boundedVec21, {});
    region::Region reg22(q, h, unboundedVec0, x0, boundedVec22, {});
    region::Region reg23(q, h, unboundedVec0, x0, boundedVec23, {});
    region::Region reg24(q, h, unboundedVec0, x0, boundedVec24, {});
    region::Region reg25(q, h, unboundedVec0, x0, boundedVec25, {});
    region::Region reg26(q, h, unboundedVec0, x0, boundedVec26, {});
    region::Region reg27(q, h, unboundedVec0, x0, boundedVec27, {});
    region::Region reg28(q, h, unboundedVec0, x0, boundedVec28, {});
    region::Region reg29(q, h, unboundedVec0, x0, boundedVec29, {});
    region::Region reg30(q, h, unboundedVec0, x0, boundedVec30, {});
    region::Region reg31(q, h, unboundedVec0, x0, boundedVec31, {});
    region::Region reg32(q, h, unboundedVec0, x0, boundedVec32, {});
    region::Region reg33(q, h, unboundedVec0, x0, boundedVec33, {});
    region::Region reg34(q, h, unboundedVec0, x0, boundedVec34, {});
    region::Region reg35(q, h, unboundedVec0, x0, boundedVec35, {});
    region::Region reg36(q, h, unboundedVec0, x0, boundedVec36, {});
    region::Region reg37(q, h, unboundedVec0, x0, boundedVec37, {});
    region::Region reg38(q, h, unboundedVec0, x0, boundedVec38, {});
    region::Region reg39(q, h, unboundedVec0, x0, boundedVec39, {});
    region::Region reg40(q, h, unboundedVec0, x0, boundedVec40, {});
    region::Region reg41(q, h, unboundedVec0, x0, boundedVec41, {});
    region::Region reg42(q, h, unboundedVec0, x0, boundedVec42, {});
    region::Region reg43(q, h, unboundedVec0, x0, boundedVec43, {});
    region::Region reg44(q, h, unboundedVec0, x0, boundedVec44, {});
    region::Region reg45(q, h, unboundedVec0, x0, boundedVec45, {});
    region::Region reg46(q, h, unboundedVec0, x0, boundedVec46, {});
    region::Region reg47(q, h, unboundedVec0, x0, boundedVec47, {});
    region::Region reg48(q, h, unboundedVec0, x0, boundedVec48, {});
    region::Region reg49(q, h, unboundedVec0, x0, boundedVec49, {});
    region::Region reg50(q, h, unboundedVec0, x0, boundedVec50, {});
    region::Region reg51(q, h, unboundedVec0, x0, boundedVec51, {});
    region::Region reg52(q, h, unboundedVec0, x0, boundedVec52, {});
    region::Region reg53(q, h, unboundedVec0, x0, boundedVec53, {});
    region::Region reg54(q, h, unboundedVec0, x0, boundedVec54, {});
    region::Region reg55(q, h, unboundedVec0, x0, boundedVec55, {});
    region::Region reg56(q, h, unboundedVec0, x0, boundedVec56, {});
    region::Region reg57(q, h, unboundedVec0, x0, boundedVec57, {});
    region::Region reg58(q, h, unboundedVec0, x0, boundedVec58, {});
    region::Region reg59(q, h, unboundedVec0, x0, boundedVec59, {});
    region::Region reg60(q, h, unboundedVec0, x0, boundedVec60, {});
    region::Region reg61(q, h, unboundedVec0, x0, boundedVec61, {});
    region::Region reg62(q, h, unboundedVec0, x0, boundedVec62, {});
    region::Region reg63(q, h, unboundedVec0, x0, boundedVec63, {});
    region::Region reg64(q, h, unboundedVec0, x0, boundedVec64, {});
    region::Region reg65(q, h, unboundedVec0, x0, boundedVec65, {});
    region::Region reg66(q, h, unboundedVec0, x0, boundedVec66, {});
    region::Region reg67(q, h, unboundedVec0, x0, boundedVec67, {});
    region::Region reg68(q, h, unboundedVec0, x0, boundedVec68, {});
    region::Region reg69(q, h, unboundedVec0, x0, boundedVec69, {});
    region::Region reg70(q, h, unboundedVec0, x0, boundedVec70, {});
    region::Region reg71(q, h, unboundedVec0, x0, boundedVec71, {});
    region::Region reg72(q, h, unboundedVec0, x0, boundedVec72, {});
    region::Region reg73(q, h, unboundedVec0, x0, boundedVec73, {});
    region::Region reg74(q, h, unboundedVec0, x0, boundedVec74, {});

    region::Region reg75(q, h, unboundedVec1, x0, boundedVec0, {});
    region::Region reg76(q, h, unboundedVec1, x0, boundedVec1, {});
    region::Region reg77(q, h, unboundedVec1, x0, boundedVec2, {});
    region::Region reg78(q, h, unboundedVec1, x0, boundedVec3, {});
    region::Region reg79(q, h, unboundedVec1, x0, boundedVec4, {});
    region::Region reg80(q, h, unboundedVec1, x0, boundedVec5, {});
    region::Region reg81(q, h, unboundedVec1, x0, boundedVec6, {});
    region::Region reg82(q, h, unboundedVec1, x0, boundedVec7, {});
    region::Region reg83(q, h, unboundedVec1, x0, boundedVec8, {});
    region::Region reg84(q, h, unboundedVec1, x0, boundedVec9, {});
    region::Region reg85(q, h, unboundedVec1, x0, boundedVec10, {});
    region::Region reg86(q, h, unboundedVec1, x0, boundedVec11, {});
    region::Region reg87(q, h, unboundedVec1, x0, boundedVec12, {});
    region::Region reg88(q, h, unboundedVec1, x0, boundedVec13, {});
    region::Region reg89(q, h, unboundedVec1, x0, boundedVec14, {});
    region::Region reg90(q, h, unboundedVec1, x0, boundedVec15, {});
    region::Region reg91(q, h, unboundedVec1, x0, boundedVec16, {});
    region::Region reg92(q, h, unboundedVec1, x0, boundedVec17, {});
    region::Region reg93(q, h, unboundedVec1, x0, boundedVec18, {});
    region::Region reg94(q, h, unboundedVec1, x0, boundedVec19, {});
    region::Region reg95(q, h, unboundedVec1, x0, boundedVec20, {});
    region::Region reg96(q, h, unboundedVec1, x0, boundedVec21, {});
    region::Region reg97(q, h, unboundedVec1, x0, boundedVec22, {});
    region::Region reg98(q, h, unboundedVec1, x0, boundedVec23, {});
    region::Region reg99(q, h, unboundedVec1, x0, boundedVec24, {});
    region::Region reg100(q, h, unboundedVec1, x0, boundedVec25, {});
    region::Region reg101(q, h, unboundedVec1, x0, boundedVec26, {});
    region::Region reg102(q, h, unboundedVec1, x0, boundedVec27, {});
    region::Region reg103(q, h, unboundedVec1, x0, boundedVec28, {});
    region::Region reg104(q, h, unboundedVec1, x0, boundedVec29, {});
    region::Region reg105(q, h, unboundedVec1, x0, boundedVec30, {});
    region::Region reg106(q, h, unboundedVec1, x0, boundedVec31, {});
    region::Region reg107(q, h, unboundedVec1, x0, boundedVec32, {});
    region::Region reg108(q, h, unboundedVec1, x0, boundedVec33, {});
    region::Region reg109(q, h, unboundedVec1, x0, boundedVec34, {});
    region::Region reg110(q, h, unboundedVec1, x0, boundedVec35, {});
    region::Region reg111(q, h, unboundedVec1, x0, boundedVec36, {});
    region::Region reg112(q, h, unboundedVec1, x0, boundedVec37, {});
    region::Region reg113(q, h, unboundedVec1, x0, boundedVec38, {});
    region::Region reg114(q, h, unboundedVec1, x0, boundedVec39, {});
    region::Region reg115(q, h, unboundedVec1, x0, boundedVec40, {});
    region::Region reg116(q, h, unboundedVec1, x0, boundedVec41, {});
    region::Region reg117(q, h, unboundedVec1, x0, boundedVec42, {});
    region::Region reg118(q, h, unboundedVec1, x0, boundedVec43, {});
    region::Region reg119(q, h, unboundedVec1, x0, boundedVec44, {});
    region::Region reg120(q, h, unboundedVec1, x0, boundedVec45, {});
    region::Region reg121(q, h, unboundedVec1, x0, boundedVec46, {});
    region::Region reg122(q, h, unboundedVec1, x0, boundedVec47, {});
    region::Region reg123(q, h, unboundedVec1, x0, boundedVec48, {});
    region::Region reg124(q, h, unboundedVec1, x0, boundedVec49, {});
    region::Region reg125(q, h, unboundedVec1, x0, boundedVec50, {});
    region::Region reg126(q, h, unboundedVec1, x0, boundedVec51, {});
    region::Region reg127(q, h, unboundedVec1, x0, boundedVec52, {});
    region::Region reg128(q, h, unboundedVec1, x0, boundedVec53, {});
    region::Region reg129(q, h, unboundedVec1, x0, boundedVec54, {});
    region::Region reg130(q, h, unboundedVec1, x0, boundedVec55, {});
    region::Region reg131(q, h, unboundedVec1, x0, boundedVec56, {});
    region::Region reg132(q, h, unboundedVec1, x0, boundedVec57, {});
    region::Region reg133(q, h, unboundedVec1, x0, boundedVec58, {});
    region::Region reg134(q, h, unboundedVec1, x0, boundedVec59, {});
    region::Region reg135(q, h, unboundedVec1, x0, boundedVec60, {});
    region::Region reg136(q, h, unboundedVec1, x0, boundedVec61, {});
    region::Region reg137(q, h, unboundedVec1, x0, boundedVec62, {});
    region::Region reg138(q, h, unboundedVec1, x0, boundedVec63, {});
    region::Region reg139(q, h, unboundedVec1, x0, boundedVec64, {});
    region::Region reg140(q, h, unboundedVec1, x0, boundedVec65, {});
    region::Region reg141(q, h, unboundedVec1, x0, boundedVec66, {});
    region::Region reg142(q, h, unboundedVec1, x0, boundedVec67, {});
    region::Region reg143(q, h, unboundedVec1, x0, boundedVec68, {});
    region::Region reg144(q, h, unboundedVec1, x0, boundedVec69, {});
    region::Region reg145(q, h, unboundedVec1, x0, boundedVec70, {});
    region::Region reg146(q, h, unboundedVec1, x0, boundedVec71, {});
    region::Region reg147(q, h, unboundedVec1, x0, boundedVec72, {});
    region::Region reg148(q, h, unboundedVec1, x0, boundedVec73, {});
    region::Region reg149(q, h, unboundedVec1, x0, boundedVec74, {});

    region::Region reg150(q, h, unboundedVec2, x0, boundedVec0, {});
    region::Region reg151(q, h, unboundedVec2, x0, boundedVec1, {});
    region::Region reg152(q, h, unboundedVec2, x0, boundedVec2, {});
    region::Region reg153(q, h, unboundedVec2, x0, boundedVec3, {});
    region::Region reg154(q, h, unboundedVec2, x0, boundedVec4, {});
    region::Region reg155(q, h, unboundedVec2, x0, boundedVec5, {});
    region::Region reg156(q, h, unboundedVec2, x0, boundedVec6, {});
    region::Region reg157(q, h, unboundedVec2, x0, boundedVec7, {});
    region::Region reg158(q, h, unboundedVec2, x0, boundedVec8, {});
    region::Region reg159(q, h, unboundedVec2, x0, boundedVec9, {});
    region::Region reg160(q, h, unboundedVec2, x0, boundedVec10, {});
    region::Region reg161(q, h, unboundedVec2, x0, boundedVec11, {});
    region::Region reg162(q, h, unboundedVec2, x0, boundedVec12, {});
    region::Region reg163(q, h, unboundedVec2, x0, boundedVec13, {});
    region::Region reg164(q, h, unboundedVec2, x0, boundedVec14, {});
    region::Region reg165(q, h, unboundedVec2, x0, boundedVec15, {});
    region::Region reg166(q, h, unboundedVec2, x0, boundedVec16, {});
    region::Region reg167(q, h, unboundedVec2, x0, boundedVec17, {});
    region::Region reg168(q, h, unboundedVec2, x0, boundedVec18, {});
    region::Region reg169(q, h, unboundedVec2, x0, boundedVec19, {});
    region::Region reg170(q, h, unboundedVec2, x0, boundedVec20, {});
    region::Region reg171(q, h, unboundedVec2, x0, boundedVec21, {});
    region::Region reg172(q, h, unboundedVec2, x0, boundedVec22, {});
    region::Region reg173(q, h, unboundedVec2, x0, boundedVec23, {});
    region::Region reg174(q, h, unboundedVec2, x0, boundedVec24, {});
    region::Region reg175(q, h, unboundedVec2, x0, boundedVec25, {});
    region::Region reg176(q, h, unboundedVec2, x0, boundedVec26, {});
    region::Region reg177(q, h, unboundedVec2, x0, boundedVec27, {});
    region::Region reg178(q, h, unboundedVec2, x0, boundedVec28, {});
    region::Region reg179(q, h, unboundedVec2, x0, boundedVec29, {});
    region::Region reg180(q, h, unboundedVec2, x0, boundedVec30, {});
    region::Region reg181(q, h, unboundedVec2, x0, boundedVec31, {});
    region::Region reg182(q, h, unboundedVec2, x0, boundedVec32, {});
    region::Region reg183(q, h, unboundedVec2, x0, boundedVec33, {});
    region::Region reg184(q, h, unboundedVec2, x0, boundedVec34, {});
    region::Region reg185(q, h, unboundedVec2, x0, boundedVec35, {});
    region::Region reg186(q, h, unboundedVec2, x0, boundedVec36, {});
    region::Region reg187(q, h, unboundedVec2, x0, boundedVec37, {});
    region::Region reg188(q, h, unboundedVec2, x0, boundedVec38, {});
    region::Region reg189(q, h, unboundedVec2, x0, boundedVec39, {});
    region::Region reg190(q, h, unboundedVec2, x0, boundedVec40, {});
    region::Region reg191(q, h, unboundedVec2, x0, boundedVec41, {});
    region::Region reg192(q, h, unboundedVec2, x0, boundedVec42, {});
    region::Region reg193(q, h, unboundedVec2, x0, boundedVec43, {});
    region::Region reg194(q, h, unboundedVec2, x0, boundedVec44, {});
    region::Region reg195(q, h, unboundedVec2, x0, boundedVec45, {});
    region::Region reg196(q, h, unboundedVec2, x0, boundedVec46, {});
    region::Region reg197(q, h, unboundedVec2, x0, boundedVec47, {});
    region::Region reg198(q, h, unboundedVec2, x0, boundedVec48, {});
    region::Region reg199(q, h, unboundedVec2, x0, boundedVec49, {});
    region::Region reg200(q, h, unboundedVec2, x0, boundedVec50, {});
    region::Region reg201(q, h, unboundedVec2, x0, boundedVec51, {});
    region::Region reg202(q, h, unboundedVec2, x0, boundedVec52, {});
    region::Region reg203(q, h, unboundedVec2, x0, boundedVec53, {});
    region::Region reg204(q, h, unboundedVec2, x0, boundedVec54, {});
    region::Region reg205(q, h, unboundedVec2, x0, boundedVec55, {});
    region::Region reg206(q, h, unboundedVec2, x0, boundedVec56, {});
    region::Region reg207(q, h, unboundedVec2, x0, boundedVec57, {});
    region::Region reg208(q, h, unboundedVec2, x0, boundedVec58, {});
    region::Region reg209(q, h, unboundedVec2, x0, boundedVec59, {});
    region::Region reg210(q, h, unboundedVec2, x0, boundedVec60, {});
    region::Region reg211(q, h, unboundedVec2, x0, boundedVec61, {});
    region::Region reg212(q, h, unboundedVec2, x0, boundedVec62, {});
    region::Region reg213(q, h, unboundedVec2, x0, boundedVec63, {});
    region::Region reg214(q, h, unboundedVec2, x0, boundedVec64, {});
    region::Region reg215(q, h, unboundedVec2, x0, boundedVec65, {});
    region::Region reg216(q, h, unboundedVec2, x0, boundedVec66, {});
    region::Region reg217(q, h, unboundedVec2, x0, boundedVec67, {});
    region::Region reg218(q, h, unboundedVec2, x0, boundedVec68, {});
    region::Region reg219(q, h, unboundedVec2, x0, boundedVec69, {});
    region::Region reg220(q, h, unboundedVec2, x0, boundedVec70, {});
    region::Region reg221(q, h, unboundedVec2, x0, boundedVec71, {});
    region::Region reg222(q, h, unboundedVec2, x0, boundedVec72, {});
    region::Region reg223(q, h, unboundedVec2, x0, boundedVec73, {});
    region::Region reg224(q, h, unboundedVec2, x0, boundedVec74, {});

    std::vector startingRegions = {
        reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9,
        reg10, reg11, reg12, reg13, reg14, reg15, reg16, reg17, reg18, reg19,
        reg20, reg21, reg22, reg23, reg24, reg25, reg26, reg27, reg28, reg29,
        reg30, reg31, reg32, reg33, reg34, reg35, reg36, reg37, reg38, reg39,
        reg40, reg41, reg42, reg43, reg44, reg45, reg46, reg47, reg48, reg49,
        reg50, reg51, reg52, reg53, reg54, reg55, reg56, reg57, reg58, reg59,
        reg60, reg61, reg62, reg63, reg64, reg65, reg66, reg67, reg68, reg69,
        reg70, reg71, reg72, reg73, reg74, reg75, reg76, reg77, reg78, reg79,
        reg80, reg81, reg82, reg83, reg84, reg85, reg86, reg87, reg88, reg89,
        reg90, reg91, reg92, reg93, reg94, reg95, reg96, reg97, reg98, reg99,
        reg100, reg101, reg102, reg103, reg104, reg105, reg106, reg107, reg108, reg109,
        reg110, reg111, reg112, reg113, reg114, reg115, reg116, reg117, reg118, reg119,
        reg120, reg121, reg122, reg123, reg124, reg125, reg126, reg127, reg128, reg129,
        reg130, reg131, reg132, reg133, reg134, reg135, reg136, reg137, reg138, reg139,
        reg140, reg141, reg142, reg143, reg144, reg145, reg146, reg147, reg148, reg149,
        reg150, reg151, reg152, reg153, reg154, reg155, reg156, reg157, reg158, reg159,
        reg160, reg161, reg162, reg163, reg164, reg165, reg166, reg167, reg168, reg169,
        reg170, reg171, reg172, reg173, reg174, reg175, reg176, reg177, reg178, reg179,
        reg180, reg181, reg182, reg183, reg184, reg185, reg186, reg187, reg188, reg189,
        reg190, reg191, reg192, reg193, reg194, reg195, reg196, reg197, reg198, reg199,
        reg200, reg201, reg202, reg203, reg204, reg205, reg206, reg207, reg208, reg209,
        reg210, reg211, reg212, reg213, reg214, reg215, reg216, reg217, reg218, reg219,
        reg220, reg221, reg222, reg223, reg224
    };

    // std::cout << "Starting from regions:" << std::endl;
    // for (const auto &reg: startingRegions)
    //     std::cout << reg.toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(startingRegions, DFS);
}


inline void testFlowerBackward2()
{
    std::cout << "Test Flower Backward 2: showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_02/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward4()
{
    std::cout << "Test Flower Backward 4: showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_04/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward6()
{
    std::cout << "Test Flower Backward 6: showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_06/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward8()
{
    std::cout << "Test Flower Backward 8: showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_08/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward10()
{
    std::cout << "Test Flower Backward 10: showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_10/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward12()
{
    std::cout << "Test Flower Backward 12: showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_12/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward14()
{
    std::cout << "Test Flower Backward 14: showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_14/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward16()
{
    std::cout << "Test Flower Backward 16: showing that from the region reached forward it is possible to go back to an initial region.\n\n";

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_16/";

    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    std::cout << "Predecessors contents:\n";
    for (const auto &region: predecessors)
        std::cout << region.toString() << std::endl;
}


int main()
{
    //std::cout << "\n--------------------\n\n";
    //
    //test0();
    //
    //std::cout << "\n--------------------\n\n";
    //
    //test1();
    //
    //std::cout << "\n\n--------------------\n\n";
    //
    //test2();
    //
    //std::cout << "\n\n--------------------\n\n";
    //
    //test3();
    //
    //std::cout << "\n\n--------------------\n\n";
    //
    //test4();
    //
    //std::cout << "\n\n--------------------\n\n";
    //
    //test5();
    //
    //std::cout << "\n\n--------------------\n\n";
    //
    //test6();
    //
    //std::cout << "\n\n--------------------\n\n";
    //
    //test7();

    testFischerFlat2();

    testFischerFlat3();

    testFischerFlat4();

    testFischerFlat5();

    testFischerFlat6();

    //testFlowerBackward2();
//
    //testFlowerBackward4();
//
    //testFlowerBackward6();
//
    //testFlowerBackward8();
//
    //testFlowerBackward10();
//
    //testFlowerBackward12();
//
    //testFlowerBackward14();
//
    //testFlowerBackward16();

    return 0;
}
