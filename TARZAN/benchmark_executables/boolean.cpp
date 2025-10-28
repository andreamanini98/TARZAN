#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testBoolean(const std::string &path, const std::string &benchmarkKey)
{
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    const int numElements = std::stoi(benchmarkKey) + 2;

    // Locations are not important here, hence each automaton is assigned std::nullopt.
    const auto goal = std::vector<std::optional<int>>(numElements, std::nullopt);

    std::vector<timed_automaton::ast::clockConstraint> intVarConstr{};
    for (int i = 1; i <= numElements; i++)
        intVarConstr.emplace_back("ctr" + std::to_string(i), EQ, 1);

    const auto res = net.forwardReachability(intVarConstr, goal, DFS);
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

    // Query: all integer variables must be equal to 1 simultaneously.
    testBoolean(path, benchmarkKey);

    return 0;
}
