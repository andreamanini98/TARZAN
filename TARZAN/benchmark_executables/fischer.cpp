#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testFischer(const std::string &path, const std::string &benchmarkKey)
{
    // Symmetry reduction (carefully handling the pid variable) should help.
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    const int numElements = std::stoi(benchmarkKey) + 2;

    const auto &locationsToInt = net.getLocationsToInt();

    // Locations are not important here, hence each automaton is assigned std::nullopt.
    auto goal = std::vector<std::optional<int>>(numElements, std::nullopt);

    goal[0] = locationsToInt[0].at("cs");
    goal[1] = locationsToInt[1].at("cs");

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

    // Query: E<> ( P(1).cs and P(2).cs ), maybe can be extended to account for every process.
    testFischer(path, benchmarkKey);

    return 0;
}
