#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testBooleanBroadcast(const std::string &path, const std::string &benchmarkKey)
{
    // The number of bits is read from the network, so the instance sizes need not form a progression.
    (void) benchmarkKey;

    const std::vector<timed_automaton::ast::timedAutomaton> automata = TARZAN::parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    int numBits = 0;
    for (const auto &automaton: automata)
        if (automaton.name.rfind("Bit", 0) == 0)
            numBits++;

    // One counter per bit: the query asks for a single instant in which every bit is set.
    std::vector<timed_automaton::ast::clockConstraint> intGoal;
    for (int i = 1; i <= numBits; i++)
        intGoal.push_back({ "ctr" + std::string(i < 10 ? "0" : "") + std::to_string(i), EQ, 1 });

    const auto goal = std::vector<std::optional<int>>(automata.size(), std::nullopt);

    const auto res = net.forwardReachability(intGoal, goal, DFS);
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

    // Query: E<> ( ctr01 == 1 && ... && ctrNN == 1 )
    testBooleanBroadcast(path, benchmarkKey);

    return 0;
}
