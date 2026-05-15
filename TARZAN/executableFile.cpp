#include <string>
#include <fstream>

#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"


int main()
{
    constexpr std::string automatonFileName = "Flower.txt";
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/models/flower/liana/flower_10/" + automatonFileName;

    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path);

    const region::RTS regionTransitionSystem(automaton);


    // ---


    // Test 1: using vector.

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachabilityWithVector({}, {}, 0, DFS);


    // ---


    // Test 2: using hash table.

    // const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);
}
