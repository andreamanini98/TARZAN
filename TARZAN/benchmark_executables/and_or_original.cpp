#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkName the name used to denote the benchmark in the logged .txt files.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testAndOrOriginal(const std::string &path, const std::string &benchmarkName, const std::string &benchmarkKey)
{
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    benchmark::benchmarkName = benchmarkName;

    // Query: compute all the state space, since the original query was A[] not...
    const std::map<std::string, std::vector<std::optional<int>>> goalLocations = {
        { "0", { 30, 30, 30 } }
    };
    const auto &goal = goalLocations.at(benchmarkKey);

    const auto res = net.forwardReachability(goal, DFS);
}


int main(const int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <path> <benchmarkName>" << std::endl;
        return 1;
    }

    const std::string path = argv[1];
    const std::string benchmarkName = argv[2];
    const std::string benchmarkKey = argv[3];

    testAndOrOriginal(path, benchmarkName, benchmarkKey);

    return 0;
}
