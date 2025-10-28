#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testVikings(const std::string &path, const std::string &benchmarkKey)
{
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    networkOfTA::RTSNetwork net(automata);

    net.enableSymmetryReduction();

    const auto &locationsToInt = net.getLocationsToInt();

    // The 3 is due to the Torch and vikings.
    const int numElements = std::stoi(benchmarkKey) + 3;

    auto goal = std::vector<std::optional<int>>(numElements, std::nullopt);
    for (int i = 1; i < numElements; i++)
        goal[i] = locationsToInt[i].at("safe");

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

    // Query: Each viking must be safe
    testVikings(path, benchmarkKey);

    return 0;
}
