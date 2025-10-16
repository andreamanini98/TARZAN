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

    const std::vector<std::optional<int>> goalLocations = { 1, 3 };

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

    const std::vector<std::optional<int>> goalLocations = { std::nullopt, std::nullopt, 1 };

    const auto res = net.forwardReachability(goalLocations, DFS);

    std::cout << res[0].toString() << std::endl;
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
            std::cout << "SUCCESSFUL parsing" << std::endl;
    } else
        std::cerr << "Wrong parsing" << std::endl;

    std::cout << ass.to_string() << std::endl;

    // --- Evaluation testing --- //

    // Variable context with initial values
    absl::btree_map<std::string, int> variables = {
        { "L", 0 },
        { "A", 10 },
        { "B", 2 }
    };

    std::cout << "\nL value before: " << variables["L"] << std::endl;

    const auto start = std::chrono::high_resolution_clock::now();

    ass.evaluate(variables);

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start);
    std::cout << "Evaluation took: " << duration.count() << " microseconds" << std::endl;

    std::cout << "Evaluated with result: " << variables["L"] << std::endl;

    std::cout << "L value after: " << variables["L"] << std::endl;
}


void testBooleanExpression()
{
    using namespace expr::ast;

    // Parse an expression from a file.
    std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/expressions/simple_boolean.txt";
    std::stringstream out;
    const std::string source = readFromFile(path);

    using parser::iterator_type;
    iterator_type iter(source.begin());
    iterator_type const end(source.end());

    booleanExpr boolE;

    // Our error handler.
    using boost::spirit::x3::with;
    using parser::error_handler_type;
    using parser::error_handler_tag;
    error_handler_type error_handler(iter, end, out, path);

    // Our parser.
    // We pass our error handler to the parser so we can access it later on in our on_error and on_success handlers.
    auto const parser = with<error_handler_tag>(std::ref(error_handler))[expr::booleanExpr()];

    // Now we parse.
    using boost::spirit::x3::ascii::space;
    // ReSharper disable once CppTooWideScope
    bool success = phrase_parse(iter, end, parser, space, boolE);

    if (success)
    {
        if (iter != end)
            error_handler(iter, "Error! Expecting end of input here: ");
        else
            std::cout << "SUCCESSFUL parsing" << std::endl;
    } else
        std::cerr << "Wrong parsing" << std::endl;

    std::cout << boolE << std::endl;

    // --- Evaluation testing --- //

    // Variable context with initial values
    absl::btree_map<std::string, int> variables = {
        { "L", 0 },
        { "A", 10 },
        { "B", 9 }
    };

    const auto start = std::chrono::high_resolution_clock::now();

    bool result = boolE.evaluate(variables);

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start);
    std::cout << "Evaluation took: " << duration.count() << " microseconds" << std::endl;

    std::cout << "Evaluated with result: " << result << std::endl;
}


void testIntegers()
{
    // const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp.txt";
    // const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path);

    // std::cout << "\n\n" << automaton << std::endl;
}


void testRingNetwork()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/ring";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [i] (6 locations): {goal -> 5, l4 -> 4, l3 -> 3, l2 -> 2, l1 -> 1, l0 -> 0}

    const std::vector<std::optional<int>> goalLocations = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };

    const auto res = net.forwardReachability(goalLocations, DFS);
}


void testVikings()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/vikings";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // soldier (4 locations): {safe -> 3, l0 -> 2, l1 -> 1, unsafe -> 0}
    // torch (4 locations): {one -> 3, two -> 2, urgent -> 1, free -> 0}

    const std::vector<std::optional<int>> goalLocations = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, std::nullopt };

    const auto res = net.forwardReachability(goalLocations, DFS);

    std::cout << res[0].toString() << std::endl;
}


void testNoClocksSingleAutomata()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    const std::string automatonFileName = "torch_no_clocks.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << regionTransitionSystem.to_string() << std::endl;

    std::cout << "\n\n\n";

    std::cout << "Computed the following regions:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(7, DFS);
}


void testNoClocksNetwork()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/test_no_clocks";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    const std::vector<std::optional<int>> goalLocations = { 3, std::nullopt };

    const auto res = net.forwardReachability(goalLocations, DFS);
}


void testFischer()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/fischer";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    //Locations:
    //    Automaton [0] (4 locations): {cs -> 3, wait -> 2, req -> 1, A -> 0}

    const std::vector<std::optional<int>> goalLocations = { 3, 3, std::nullopt, std::nullopt, std::nullopt };

    const auto res = net.forwardReachability(goalLocations, DFS);
}


void test_exSITH()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/";

    const std::string automatonFileName = "exSITH.txt";

    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    std::cout << regionTransitionSystem.to_string() << std::endl;

    // Locations (5):
    // q0 -> 4
    // q1 -> 3
    // q2 -> 2
    // q3 -> 1
    // qBad -> 0

    std::cout << "Computed the following regions:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(1, DFS);

    for (const auto &region: rts) std::cout << region.toString() << std::endl;
}


void test_rcp()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/rcp";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [0] (11 locations): {N1almost_child -> 10, N1rec_idle_slow -> 9, N1rec_idle_fast -> 8, N1rec_idle -> 7, N1root -> 6, N1child -> 5, N1almost_root -> 4, N1snt_req -> 3, N1rec_req_slow -> 2, N1rec_req_fast -> 1, N1root_contention -> 0}
    // Automaton [1] (11 locations): {N2almost_child -> 10, N2rec_idle_slow -> 9, N2snt_req -> 8, N2rec_req_fast -> 7, N2almost_root -> 6, N2rec_idle_fast -> 5, N2rec_req_slow -> 4, N2root -> 2, N2child -> 3, N2rec_idle -> 1, N2root_contention -> 0}
    // Automaton [2] (6 locations): {S1o4 -> 5, S1o2 -> 4, S1o1 -> 2, S1oEnd -> 3, S1o3 -> 1, S1oStart -> 0}
    // Automaton [3] (10 locations): {W12rec_ack_req -> 9, W12rec_req_ack -> 8, W12rec_req_idle -> 7, W12rec_ack_idle -> 6, W12rec_req -> 5, W12rec_idle -> 4, W12rec_ack -> 1, W12rec_idle_ack -> 3, W12rec_idle_req -> 2, W12empty -> 0}
    // Automaton [4] (10 locations): {W21rec_req_idle -> 9, W21rec_idle_ack -> 8, W21rec_ack_req -> 7, W21rec_req_ack -> 6, W21rec_idle_req -> 5, W21rec_req -> 4, W21rec_ack_idle -> 3, W21rec_idle -> 2, W21rec_ack -> 1, W21empty -> 0}

    const std::vector<std::optional<int>> goalLocations = { std::nullopt, std::nullopt, 3, std::nullopt, std::nullopt };
    const auto res = net.forwardReachability(goalLocations, DFS);
    //std::cout << res[0].toString() << std::endl;
}


void test_AndOr_original()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/AndOr_original";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [0] (8 locations): {And111 -> 7, And110 -> 6, And101 -> 5, And100 -> 4, And011 -> 3, And010 -> 2, And001 -> 1, And000 -> 0}
    // Automaton [1] (4 locations): {Input4 -> 3, Input3 -> 2, Input2 -> 1, InputInit -> 0}
    // Automaton [2] (8 locations): {Or110 -> 7, Or101 -> 6, Or111 -> 5, Or011 -> 4, Or100 -> 3, Or010 -> 2, Or001 -> 1, Or000 -> 0}

    const std::vector<std::optional<int>> goalLocations = { 2, 3, 4 };
    const auto res = net.forwardReachability(goalLocations, DFS);
    //std::cout << res[0].toString() << std::endl;
}


void test_csma_20n()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/csma_20N";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    // Locations:
    // Automaton [0] (22 locations): {bus_collision17 -> 21, bus_collision20 -> 20, bus_collision15 -> 19, bus_collision14 -> 18, bus_collision19 -> 17, bus_collision12 -> 16, bus_collision10 -> 15, bus_collision2 -> 14, bus_collision9 -> 13, bus_collision8 -> 12, bus_idle -> 11, bus_collision6 -> 10, bus_collision7 -> 9, bus_collision5 -> 8, bus_collision4 -> 7, bus_collision11 -> 6, bus_collision3 -> 5, bus_active -> 0, bus_collision18 -> 3, bus_collision16 -> 2, bus_collision13 -> 1, bus_collision1 -> 4}
    // Automaton [1] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [2] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [3] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [4] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [5] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [6] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [7] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [8] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [9] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [10] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [11] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [12] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [13] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [14] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [15] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [16] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [17] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [18] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [19] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}
    // Automaton [20] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}

    std::cout << net.toString() << std::endl;

    const std::vector<std::optional<int>> goalLocations = {
        std::nullopt,
        2,
        2,
        1,
        2,
        2,
        2,
        2,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt
    };

    const auto res = net.forwardReachability(goalLocations, DFS);
}


int main()
{
#ifdef REGION_TIMING
    std::cout << "Tick period: " << static_cast<double>(std::chrono::high_resolution_clock::period::num) / std::chrono::high_resolution_clock::period::den <<
            " seconds\n";
    const auto start = std::chrono::high_resolution_clock::now();
#endif


    test_csma_20n();


#ifdef REGION_TIMING
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;
#endif

    return 0;
}
