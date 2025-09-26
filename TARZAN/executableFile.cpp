#include "regions/networkOfTA/RTSNetwork.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/parser/input_output_action_enum.h"
#include "TARZAN/utilities/partition_utilities.h"

// #define REGION_TIMING


void testNetworkActions()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";

    const std::string automatonFileName = "light_switch.txt";

    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    std::cout << "\n\n\n\n\n";
    std::cout << "Parsed automaton: " << automaton << std::endl;

    std::cout << "\nActions:" << std::endl;
    for (const auto &action: automaton.actions)
        std::cout << action << std::endl;

    std::cout << "\nNow printing the actions in transitions:" << std::endl;
    for (const auto &transition: automaton.transitions)
    {
        std::cout << transition << std::endl;
        std::cout << "Action first element: " << transition.action.first << std::endl;
        std::cout << "Action second element: " <<
                (transition.action.second.has_value() ? in_out_act_to_string(transition.action.second.value()) : "NO_VALUE") << std::endl;
        std::cout << std::endl;
    }
}


void testParseMultipleAutomata()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/TA_semantics_algorithms_tools";

    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);

    for (const auto &automaton: automata)
        std::cout << "\nParsed the following automaton:\n" << automaton << std::endl;

    const networkOfTA::RTSNetwork net(automata);

    std::cout << "\n\n\n\n\n";
    std::cout << "Network:" << std::endl;
    std::cout << net.toString() << std::endl;
}


void testGetImmediateNetworkDelaySuccessor()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/TA_semantics_algorithms_tools";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << "\n\n\n\n\n";
    std::cout << "Network:" << std::endl;
    std::cout << net.toString() << std::endl;

    const auto &initialRegions = net.getInitialRegions();

    std::cout << "\n\n\n\n\n\n";
    std::cout << "Starting from the following Network Regions:\n" << std::endl;
    for (const auto &region: initialRegions)
        std::cout << region.toString() << std::endl;

    std::cout << "\n\n\n";

    networkOfTA::NetworkRegion current = initialRegions[0];
    for (int i = 1; i <= 40; i++)
    {
        current = current.getImmediateDelaySuccessor(net.getMaxConstants());
        std::cout << "Successor " << i << ":\n" << current.toString() << std::endl;
    }
}


int main()
{
#ifdef REGION_TIMING
    std::cout << "Tick period: " << static_cast<double>(std::chrono::high_resolution_clock::period::num) / std::chrono::high_resolution_clock::period::den <<
            " seconds\n";
    const auto start = std::chrono::high_resolution_clock::now();
#endif


    testGetImmediateNetworkDelaySuccessor();


#ifdef REGION_TIMING
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;
#endif

    return 0;
}
