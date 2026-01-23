#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testLynch(const std::string &path, const std::string &benchmarkKey)
{
    const std::vector<timed_automaton::ast::timedAutomaton> automata = TARZAN::parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    const int numElements = std::stoi(benchmarkKey) + 2;

    const auto &locationsToInt = net.getLocationsToInt();

    std::vector<std::optional<int>> goal(numElements, std::nullopt);
    goal[0] = locationsToInt[0].at("CS7");
    goal[1] = locationsToInt[1].at("CS7");

    const auto res = net.forwardReachability(goal, DFS);
}


int main(const int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <path> <key>" << std::endl;
        return 1;
    }

    const std::string path = argv[1];
    const std::string benchmarkKey = argv[2];

    // Query: E<> P(1).CS7 && P(2).CS7
    testLynch(path, benchmarkKey);

    return 0;
}
