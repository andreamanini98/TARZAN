#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testFlipflop(const std::string &path, const std::string &benchmarkKey)
{
    (void) benchmarkKey;

    const std::vector<timed_automaton::ast::timedAutomaton> automata = TARZAN::parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    const auto &locationsToInt = net.getLocationsToInt();

    auto goal = std::vector<std::optional<int>>(automata.size(), std::nullopt);
    auto clockGoal = std::vector<std::vector<timed_automaton::ast::clockConstraint>>(automata.size());

    for (int i = 0; i < static_cast<int>(automata.size()); i++)
        if (automata[i].name == "input")
        {
            goal[i] = locationsToInt[i].at("Input3");
            clockGoal[i].push_back({ "s", LE, 30 });
        }

    const std::vector<timed_automaton::ast::clockConstraint> intGoal = { { "qLevel", EQ, 1 } };

    const auto res = net.forwardReachability(intGoal, clockGoal, goal, DFS);
    (void) res;
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

    // Query: E<> ( input.Input3 && qLevel == 1 && input.s <= 30 )
    testFlipflop(path, benchmarkKey);

    return 0;
}
