#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/utilities/file_utilities.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 */
inline void testSimple(const std::string &path)
{
    const std::string automatonFileName = "Template.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    // Placeholder value, since we explore the entire state space.
    constexpr int goal = 30;

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

    // Query: explore the entire state space.
    testSimple(path);

    return 0;
}
