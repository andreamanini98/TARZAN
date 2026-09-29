#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/networkOfTA/RTSNetwork.h"

#include <filesystem>


/**
 * @param path the path to the directory containing all benchmark subdirectories.
 * @param benchmarkKey a string used to retrieve the necessary auxiliary data for the benchmark at hand.
 */
inline void testClockbankCommitted(const std::string &path, const std::string &benchmarkKey)
{
    (void) benchmarkKey;

    const std::vector<timed_automaton::ast::timedAutomaton> automata = TARZAN::parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    // The tuned (number of clocks, radix of the low digit) pairs: the query target of the low
    // digit is the radix minus one, so it is read from the instance name.
    std::filesystem::path folder(path);
    if (folder.filename().empty())
        folder = folder.parent_path();
    const std::string instance = folder.filename().string();

    int lowDigitMax = 0;
    if (instance == "clockbankCommitted_85")
        lowDigitMax = 7;
    else if (instance == "clockbankCommitted_90")
        lowDigitMax = 6;
    else
    {
        std::cerr << "Unknown instance: " << instance << std::endl;
        return;
    }

    // The odometer has run through every value of its four digits.
    const std::vector<timed_automaton::ast::clockConstraint> intGoal = {
        { "d1", EQ, lowDigitMax }, { "d2", EQ, 9 }, { "d3", EQ, 9 }, { "d4", EQ, 9 } };

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

    // Query: E<> ( d1 == lowDigitMax && d2 == 9 && d3 == 9 && d4 == 9 )
    testClockbankCommitted(path, benchmarkKey);

    return 0;
}
