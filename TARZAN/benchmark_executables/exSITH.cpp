#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/utilities/file_utilities.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 */
inline void testExSITH(const std::string &path)
{
    const std::string automatonFileName = "exSITH.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    const auto &locToIntMap = regionTransitionSystem.getLocationsToInt();

    const int goal = locToIntMap.at("qBad");

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

    // Query: A[] not A.qBad, equivalent to: E<> qBad
    testExSITH(path);

    return 0;
}
