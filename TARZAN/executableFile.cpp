#include "regions/networkOfTA/RTSNetwork.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "parser/enums/input_output_action_enum.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/parser/timed_automaton_def.h"
#include "TARZAN/parser/config.h"

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


void testNetworkBuildRegionGraphForeword()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/TA_semantics_algorithms_tools";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [0] (3 locations): {dim -> 2, bright -> 1, off -> 0}
    // Automaton [1] (4 locations): {relax -> 3, t -> 2, study -> 1, idle -> 0}

    const std::vector goalLocations = { 1, 3 };

    // Reachable.
    // const std::vector goalLocations = { 1, 1 };

    // Unreachable (the entire region graph will be computed).
    // const std::vector goalLocations = { 1, 3 };

    const auto res = net.forwardReachability(goalLocations, DFS);
}


void testNetworkTrainGateController()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/baier_train_gate_controller";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [0] (3 locations): {in -> 2, near -> 1, far -> 0}
    // Automaton [1] (4 locations): {3 -> 3, 2 -> 2, 1 -> 1, 0 -> 0}
    // Automaton [2] (4 locations): {going_up -> 3, down -> 2, coming_down -> 1, up -> 0}

    const std::vector goalLocations = { 2, 1, 0 };

    const auto res = net.forwardReachability(goalLocations, DFS);
}


void testParseInvariants()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";

    const std::string automatonFileName = "light_switch.txt";

    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    std::cout << "\n\n\n\n\n";
    std::cout << "Parsed automaton: " << automaton << std::endl;
}


void testMultipleMaxConstants()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    const std::string automatonFileName = "test_flower_small.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const auto &clocksIndices = automaton.getClocksIndices();
    const auto &maxConstants = automaton.getMaxConstants(clocksIndices);

    std::cout << "Clock indices:" << std::endl;
    for (const auto &[fst, snd]: clocksIndices)
        std::cout << fst << " " << snd + 1 << std::endl;

    std::cout << "Maximum constants:" << std::endl;
    for (int i = 0; i < static_cast<int>(maxConstants.size()); i++)
        std::cout << i + 1 << " " << maxConstants[i] << std::endl;
}


void testExpression()
{
    using namespace expr::ast;

    // --- Manual testing --- //

    // Build a small expression by hand: L = L + 1 * (3 + N).
    assignmentExpr a;
    a.lhs.name = "L";
    a.rhs = binaryExpr{
        variable{ "L" },
        ADD,
        binaryExpr{
            1,
            MUL,
            binaryExpr{ 3, ADD, variable{ "N" } }
        }
    };

    std::cout << a.to_string() << "\n";

    // -- Parse testing --- //

    // Parse an expression from a file.
    std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/expressions/simple_assignment.txt";
    std::stringstream out;
    const std::string source = readFromFile(path);

    using parser::iterator_type;
    iterator_type iter(source.begin());
    iterator_type const end(source.end());

    assignmentExpr ass;

    // Our error handler.
    using boost::spirit::x3::with;
    using parser::error_handler_type;
    using parser::error_handler_tag;
    error_handler_type error_handler(iter, end, out, path);

    // Our parser.
    // We pass our error handler to the parser so we can access it later on in our on_error and on_success handlers.
    auto const parser = with<error_handler_tag>(std::ref(error_handler))[expr::assignmentExpr()];

    // Now we parse.
    using boost::spirit::x3::ascii::space;
    // ReSharper disable once CppTooWideScope
    bool success = phrase_parse(iter, end, parser, space, ass);

    if (success)
    {
        if (iter != end)
            error_handler(iter, "Error! Expecting end of input here: ");
        else
            std::cerr << "SUCCESSFUL parsing" << std::endl;
    } else
        std::cerr << "Wrong parsing" << std::endl;

    std::cout << ass.to_string() << std::endl;

    // --- Evaluation testing --- //

    // Variable context with initial values
    absl::flat_hash_map<std::string, int> variables = {
        {"L", 0},
        { "A", 10 },
        { "B", 2 }
    };

    std::cout << "\nL value before: " << variables["L"] << std::endl;

    const auto start = std::chrono::high_resolution_clock::now();

    int result = ass.evaluate(variables);

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start);
    std::cout << "Evaluation took: " << duration.count() << " microseconds" << std::endl;

    std::cout << "Evaluated with result: " << result << std::endl;

    std::cout << "L value after: " << variables["L"] << std::endl;
}


int main()
{
#ifdef REGION_TIMING
    std::cout << "Tick period: " << static_cast<double>(std::chrono::high_resolution_clock::period::num) / std::chrono::high_resolution_clock::period::den <<
            " seconds\n";
    const auto start = std::chrono::high_resolution_clock::now();
#endif


    testExpression();


#ifdef REGION_TIMING
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;
#endif

    return 0;
}
