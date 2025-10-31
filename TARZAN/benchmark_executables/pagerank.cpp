#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 */
inline void testPagerank(const std::string &path)
{
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    const auto &locationsToInt = net.getLocationsToInt();

    std::vector<std::optional<int>> goal(9, std::nullopt);
    goal[7] = locationsToInt[7].at("Completed");

    // Using 1300 as the comparing constant. Values lower than 1256 will cause an explosion.
    // std::vector<std::vector<timed_automaton::ast::clockConstraint>> goalClockConstraints(9);
    // goalClockConstraints[0] = { { "timer", LT, 1300 } };

    const auto res = net.forwardReachability(goal, DFS);
}


int main(const int argc, char *argv[])
{
    if (argc != 3)
    {
        // Key parameter automatically inserted by scripts but ignored here.
        std::cerr << "Usage: " << argv[0] << " <path> <key>" << std::endl;
        return 1;
    }

    const std::string path = argv[1];

    // Query: E<> stage7.Completed
    testPagerank(path);

    return 0;
}
