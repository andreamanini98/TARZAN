#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testMpeg2(const std::string &path, const std::string &benchmarkKey)
{
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    const auto &locationsToInt = net.getLocationsToInt();

    const int numBFrames = std::stoi(benchmarkKey) * 4 + 4;
    const int numElements = numBFrames + 4;

    auto goal = std::vector<std::optional<int>>(numElements, std::nullopt);
    for (int i = 0; i < numElements - 4; i++)
        goal[i] = locationsToInt[i].at("Bout");

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

    // Query: E<> ( every BFrame in Bout )
    testMpeg2(path, benchmarkKey);

    return 0;
}
