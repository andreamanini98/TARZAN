#ifndef TARZAN_FLOWER_BACKWARD_H
#define TARZAN_FLOWER_BACKWARD_H

#include <string>
#include <fstream>

#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"


inline void testFlowerBackward2()
{
    std::cout << "Flower K = 2" << std::endl;
    std::cout << "------------\n\n";

    const std::string path = "../../TARZAN/benchmarks/models/flower/liana/flower_02/";

    constexpr std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    // std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    // std::cout << "Predecessors contents:\n";
    // for (const auto &region: predecessors)
    //     std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward4()
{
    std::cout << "Flower K = 4" << std::endl;
    std::cout << "------------\n\n";

    const std::string path = "../../TARZAN/benchmarks/models/flower/liana/flower_04/";

    constexpr std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    // std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    // std::cout << "Predecessors contents:\n";
    // for (const auto &region: predecessors)
    //     std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward6()
{
    std::cout << "Flower K = 6" << std::endl;
    std::cout << "------------\n\n";

    const std::string path = "../../TARZAN/benchmarks/models/flower/liana/flower_06/";

    constexpr std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    // std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    // std::cout << "Predecessors contents:\n";
    // for (const auto &region: predecessors)
    //     std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward8()
{
    std::cout << "Flower K = 8" << std::endl;
    std::cout << "------------\n\n";

    const std::string path = "../../TARZAN/benchmarks/models/flower/liana/flower_08/";

    constexpr std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    // std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    // std::cout << "Predecessors contents:\n";
    // for (const auto &region: predecessors)
    //     std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward10()
{
    std::cout << "Flower K = 10" << std::endl;
    std::cout << "-------------\n\n";

    const std::string path = "../../TARZAN/benchmarks/models/flower/liana/flower_10/";

    constexpr std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    // std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    // std::cout << "Predecessors contents:\n";
    // for (const auto &region: predecessors)
    //     std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward12()
{
    std::cout << "Flower K = 12" << std::endl;
    std::cout << "-------------\n\n";

    const std::string path = "../../TARZAN/benchmarks/models/flower/liana/flower_12/";

    constexpr std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    // std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    // std::cout << "Predecessors contents:\n";
    // for (const auto &region: predecessors)
    //     std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward14()
{
    std::cout << "Flower K = 14" << std::endl;
    std::cout << "-------------\n\n";

    const std::string path = "../../TARZAN/benchmarks/models/flower/liana/flower_14/";

    constexpr std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    // std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    // std::cout << "Predecessors contents:\n";
    // for (const auto &region: predecessors)
    //     std::cout << region.toString() << std::endl;
}


inline void testFlowerBackward16()
{
    std::cout << "Flower K = 16" << std::endl;
    std::cout << "------------\n\n";

    const std::string path = "../../TARZAN/benchmarks/models/flower/liana/flower_16/";

    constexpr std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << "Forward computation output:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    std::cout << '\n';

    // std::cout << "Starting from region:\n" << rts[0].toString() << std::endl;

    std::cout << "Backward computation output:" << std::endl;

    const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);

    // std::cout << "Predecessors contents:\n";
    // for (const auto &region: predecessors)
    //     std::cout << region.toString() << std::endl;
}

#endif //TARZAN_FLOWER_BACKWARD_H
