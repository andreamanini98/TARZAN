#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/utilities/file_utilities.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkName the name used to denote the benchmark in the logged .txt files.
 */
inline void testFlower(const std::string &path, const std::string &benchmarkName)
{
    const std::string automatonFileName = "Flower.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    benchmark::benchmarkName = benchmarkName;

    // Query: it is possible to reach the goal location.
    constexpr int targetLocation = 0;

    const region::RTS regionTransitionSystem(automaton);

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(targetLocation, DFS);
}


int main(const int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <path> <benchmarkName>" << std::endl;
        return 1;
    }

    const std::string path = argv[1];
    const std::string benchmarkName = argv[2];

    testFlower(path, benchmarkName);

    return 0;
}
