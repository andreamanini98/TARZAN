#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testCsma(const std::string &path, const std::string &benchmarkKey)
{
    // Symmetry reduction cannot be applied as is due to the different names in the channels.
    const std::vector<timed_automaton::ast::timedAutomaton> automata = TARZAN::parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    const auto &locationsToInt = net.getLocationsToInt();

    const std::map<std::string, int> keyToNetworkSize{
        { "0", 21 },
        { "1", 23 },
        { "2", 26 },
        { "3", 31 },
        { "4", 51 }
    };

    std::vector<std::optional<int>> goal(keyToNetworkSize.at(benchmarkKey), std::nullopt);
    goal[1] = locationsToInt[1].at("sender_retry");
    goal[2] = locationsToInt[2].at("sender_retry");
    goal[3] = locationsToInt[3].at("sender_transm");
    goal[4] = locationsToInt[4].at("sender_retry");
    goal[5] = locationsToInt[5].at("sender_retry");
    goal[6] = locationsToInt[6].at("sender_retry");
    goal[7] = locationsToInt[7].at("sender_retry");

    // With this additional constraint (P3.x >=52), it explodes.
    // std::vector<std::vector<timed_automaton::ast::clockConstraint>> goalClockConstraints(keyToNetworkSize.at(benchmarkKey));
    // goalClockConstraints[3] = { { "x", GE, 52 } };
    // const auto res = net.forwardReachability(goalClockConstraints, goal, DFS);

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

    // Query: E<> P1.sender_retry && P2.sender_retry && P3.sender_transm && P3.x >= 52 && P4.sender_retry && P5.sender_retry && P6.sender_retry && P7.sender_retry
    testCsma(path, benchmarkKey);

    return 0;
}
