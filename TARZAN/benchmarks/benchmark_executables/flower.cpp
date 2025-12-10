#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/utilities/file_utilities.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 */
inline void testFlower(const std::string &path)
{
    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = TARZAN::parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("Goal");

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(goal, DFS);
}


int main(const int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <path>" << std::endl;
        return 1;
    }

    const std::string path = argv[1];

    // Query: it is possible to reach the goal location.
    testFlower(path);

    return 0;
}
