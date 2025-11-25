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

    std::cout << "Starting from regions:" << std::endl;
    for (const auto &reg: startingRegions)
        std::cout << reg.toString() << std::endl;

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

    std::cout << "Starting from regions:" << std::endl;
    for (const auto &reg: startingRegions)
        std::cout << reg.toString() << std::endl;

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

    std::cout << "Starting from regions:" << std::endl;
    for (const auto &reg: startingRegions)
        std::cout << reg.toString() << std::endl;

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

    std::deque<boost::dynamic_bitset<>> unboundedVec;
    unboundedVec.push_front(unbounded0);
    unboundedVec.push_front(unbounded1);

    boost::dynamic_bitset<> bounded0(5);
    bounded0[2] = true;
    bounded0[3] = true;
    bounded0[4] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec;
    boundedVec.push_front(bounded0);

    boost::dynamic_bitset<> x0(5);

    region::Region reg(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg };

    std::cout << "Starting from region:\n" << reg.toString() << std::endl;

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

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("req1_req2_req3_req4_cs5_cs6_id6");

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

    std::deque<boost::dynamic_bitset<>> unboundedVec;
    unboundedVec.push_front(unbounded0);
    unboundedVec.push_front(unbounded1);

    boost::dynamic_bitset<> bounded0(6);
    bounded0[2] = true;
    bounded0[3] = true;
    bounded0[4] = true;
    bounded0[5] = true;

    std::deque<boost::dynamic_bitset<>> boundedVec;
    boundedVec.push_front(bounded0);

    boost::dynamic_bitset<> x0(6);

    region::Region reg(q, h, unboundedVec, x0, boundedVec, {});

    std::vector startingRegions = { reg };

    std::cout << "Starting from region:\n" << reg.toString() << std::endl;

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

    //testFischerFlat2();
    //
    //testFischerFlat3();
    //
    testFischerFlat4();
    //
    //testFischerFlat5();
    //
    //testFischerFlat6();

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
