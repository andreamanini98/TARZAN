#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testMedicalWorkflowBroadcast(const std::string &path, const std::string &benchmarkKey)
{
    // The goal is located by automaton name, so the broadcast variant (which carries an extra Init
    // automaton for the non-zero initial values) and the point-to-point one can share this runner.
    (void) benchmarkKey;

    const std::vector<timed_automaton::ast::timedAutomaton> automata = TARZAN::parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    const auto &locationsToInt = net.getLocationsToInt();

    auto goal = std::vector<std::optional<int>>(automata.size(), std::nullopt);
    for (int i = 0; i < static_cast<int>(automata.size()); i++)
        if (automata[i].name.rfind("Patient", 0) == 0)
            goal[i] = locationsToInt[i].at("Done");

    const auto res = net.forwardReachability(goal, DFS);
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

    // Query: E<> ( every Patient in Done )
    testMedicalWorkflowBroadcast(path, benchmarkKey);

    return 0;
}
