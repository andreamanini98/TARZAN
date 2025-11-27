#ifndef TARZAN_GENERIC_BACKWARD_TESTS_H
#define TARZAN_GENERIC_BACKWARD_TESTS_H

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


inline void test4()
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


inline void test5()
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


inline void test6()
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

#endif //TARZAN_GENERIC_BACKWARD_TESTS_H
