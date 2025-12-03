#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testTrainAHV93(const std::string &path, const std::string &benchmarkKey)
{
    const std::vector<timed_automaton::ast::timedAutomaton> automata = TARZAN::parseTimedAutomataFromFolder(path);
    networkOfTA::RTSNetwork net(automata);

    net.enableSymmetryReduction();

    const auto &locationsToInt = net.getLocationsToInt();

    // benchmarkKey * 4 + 4 should match the actual number of trains in the directory (since automata are sorted alphabetically and each Train is capitalized).
    // Total automata = num_trains + controller + gate
    const int numTrains = std::stoi(benchmarkKey) * 4 + 4;
    const int numElements = numTrains + 2;

    auto goal = std::vector<std::optional<int>>(numElements, std::nullopt);
    // Controller is at index numTrains (after all trains, before gate alphabetically).
    goal[numTrains] = locationsToInt[numTrains].at("controller3");

    // With the check on the counter, it explodes.
    // std::vector<timed_automaton::ast::clockConstraint> intVarConstr{};
    // intVarConstr.emplace_back("cnt", GT, 0);
    // const auto res = net.forwardReachability(intVarConstr, goal, BFS);

    // Without the integer, the BFS exploration technique is faster in this example.
    const auto res = net.forwardReachability(goal, BFS);
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

    // Query: A[] not (controller.controller3 && cnt > 0) now expressed as the equivalent reachability
    testTrainAHV93(path, benchmarkKey);

    return 0;
}
