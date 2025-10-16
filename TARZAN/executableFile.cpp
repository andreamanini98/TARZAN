#include "regions/networkOfTA/RTSNetwork.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "parser/enums/input_output_action_enum.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/utilities/file_utilities.h"

// #define REGION_TIMING


inline void testFlower()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    // const std::string automatonFileName = "light_switch.txt";

    const std::string automatonFileName = "test_flower_small.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    region::RTS regionTransitionSystem(automaton);

    //std::cout << "Location to Integer Mapping:\n";
    //for (const auto& [location, index] : automaton.mapLocationsToInt()) {
    //    std::cout << location << " -> " << index << std::endl;
    //}

    std::cout << "\n\n\n";

    std::cout << "Computed the following regions:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);

    for (const auto &region: rts)
        std::cout << region.toString() << std::endl;
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


void testFischer()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/fischer";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    //Locations:
    //    Automaton [0] (4 locations): {cs -> 3, wait -> 2, req -> 1, A -> 0}

    const std::vector<std::optional<int>> goalLocations = { 2, 2, 2, 2, 2, 2 };

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

    const std::vector<std::optional<int>> goalLocations = { std::nullopt, 3, std::nullopt };
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
    // Automaton [1...20] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}

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


void test_csma_50n()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/csma_50N";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    // Locations:
    // Automaton [0] (22 locations): {bus_collision17 -> 21, bus_collision20 -> 20, bus_collision15 -> 19, bus_collision14 -> 18, bus_collision19 -> 17, bus_collision12 -> 16, bus_collision10 -> 15, bus_collision2 -> 14, bus_collision9 -> 13, bus_collision8 -> 12, bus_idle -> 11, bus_collision6 -> 10, bus_collision7 -> 9, bus_collision5 -> 8, bus_collision4 -> 7, bus_collision11 -> 6, bus_collision3 -> 5, bus_active -> 0, bus_collision18 -> 3, bus_collision16 -> 2, bus_collision13 -> 1, bus_collision1 -> 4}
    // Automaton [1...50] (3 locations): {sender_retry -> 2, sender_transm -> 1, sender_wait -> 0}

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


void test_maler()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/maler";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [0] (9 locations): {End1 -> 8, P1 -> 7, Q1 -> 6, N1 -> 5, L1 -> 4, K1 -> 3, J1 -> 2, M1 -> 1, I1 -> 0}
    // Automaton [1] (9 locations): {P2 -> 8, N2 -> 7, Q2 -> 6, L2 -> 5, M2 -> 4, J2 -> 3, I2 -> 0, End2 -> 1, K2 -> 2}
    // Automaton [2] (9 locations): {End3 -> 8, P3 -> 7, N3 -> 6, M3 -> 5, Q3 -> 4, K3 -> 3, J3 -> 2, L3 -> 1, I3 -> 0}
    // Automaton [3] (9 locations): {End4 -> 8, Q4 -> 7, N4 -> 6, M4 -> 5, L4 -> 4, K4 -> 3, J4 -> 1, P4 -> 2, I4 -> 0}

    const std::vector<std::optional<int>> goalLocations = { 8, 1, 8, 8 };
    const auto res = net.forwardReachability(goalLocations, DFS);
}


void test_soldiers()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/soldiers";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [0] (2 locations): {Escape -> 1, Wait -> 0}
    // Automaton [1] (4 locations): {OnWayBack -> 3, OnBridge -> 2, Safety -> 1, Peril -> 0}
    // Automaton [2] (4 locations): {OnWayBack -> 3, OnBridge -> 2, Safety -> 1, Peril -> 0}
    // Automaton [3] (4 locations): {OnWayBack -> 3, OnBridge -> 2, Safety -> 1, Peril -> 0}
    // Automaton [4] (4 locations): {OnWayBack -> 3, OnBridge -> 2, Safety -> 1, Peril -> 0}

    const std::vector<std::optional<int>> goalLocations = { 1, std::nullopt, std::nullopt, std::nullopt, std::nullopt };
    const auto res = net.forwardReachability(goalLocations, DFS);
}


void test_SRlatch()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/SRlatch";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [0] (3 locations): {env_final -> 2, env_10 -> 1, env_11 -> 0}
    // Automaton [1] (8 locations): {Nor1_110 -> 7, Nor1_101 -> 6, Nor1_111 -> 5, Nor1_000 -> 4, Nor1_010 -> 3, Nor1_001 -> 2, Nor1_100 -> 1, Nor1_011 -> 0}
    // Automaton [2] (8 locations): {Nor2_110 -> 7, Nor2_101 -> 6, Nor2_011 -> 5, Nor2_010 -> 4, Nor2_100 -> 3, Nor2_001 -> 1, Nor2_111 -> 2, Nor2_000 -> 0}

    const std::vector<std::optional<int>> goalLocations = { 23, std::nullopt, std::nullopt };
    const auto res = net.forwardReachability(goalLocations, DFS);
}


void test_latch()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/latch";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [0] (5 locations): {AndHigh1 -> 4, AndLow2 -> 3, AndHigh1bis -> 2, AndLow1bis -> 1, AndLow1 -> 0}
    // Automaton [1] (2 locations): {ClockHigh1 -> 1, ClockLow1 -> 0}
    // Automaton [2] (4 locations): {DLow2 -> 3, DHigh1 -> 2, DHigh2 -> 1, DLow1 -> 0}
    // Automaton [3] (5 locations): {LatchD1E1B -> 4, LatchD1E0 -> 3, LatchD0E1 -> 2, LatchD1E1 -> 1, LatchD0E0 -> 0}
    // Automaton [4] (3 locations): {Not1High1bis -> 2, Not1Low1 -> 1, Not1High1 -> 0}
    // Automaton [5] (3 locations): {Not2High1 -> 2, Not2Low1bis -> 1, Not2Low1 -> 0}
    // Automaton [6] (5 locations): {XorLow2 -> 4, XorHigh1bis -> 3, XorHigh1 -> 2, XorLow1bis -> 1, XorLow1 -> 0}

    const std::vector<std::optional<int>> goalLocations = { std::nullopt, std::nullopt, 2, std::nullopt, std::nullopt, std::nullopt, std::nullopt };
    const auto res = net.forwardReachability(goalLocations, DFS);
}


void test_lynch_4_16()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp/lynch_4_16";
    const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
    const networkOfTA::RTSNetwork net(automata);

    std::cout << net.toString() << std::endl;

    // Locations:
    // Automaton [0] (9 locations): {L3 -> 8, L2 -> 7, CS7 -> 6, L5 -> 5, L6 -> 4, L8 -> 3, L1 -> 2, L4 -> 1, L9 -> 0}
    // Automaton [1] (9 locations): {L3 -> 8, L2 -> 7, CS7 -> 6, L5 -> 5, L6 -> 4, L8 -> 3, L1 -> 2, L4 -> 1, L9 -> 0}
    // Automaton [2] (9 locations): {L3 -> 8, L2 -> 7, CS7 -> 6, L5 -> 5, L6 -> 4, L8 -> 3, L1 -> 2, L4 -> 1, L9 -> 0}
    // Automaton [3] (9 locations): {L3 -> 8, L2 -> 7, CS7 -> 6, L5 -> 5, L6 -> 4, L8 -> 3, L1 -> 2, L4 -> 1, L9 -> 0}

    const std::vector<std::optional<int>> goalLocations = { 6, 6, std::nullopt, std::nullopt };
    const auto res = net.forwardReachability(goalLocations, DFS);
}


int main()
{
#ifdef REGION_TIMING
    std::cout << "Tick period: " << static_cast<double>(std::chrono::high_resolution_clock::period::num) / std::chrono::high_resolution_clock::period::den <<
            " seconds\n";
    const auto start = std::chrono::high_resolution_clock::now();
#endif


    test_lynch_4_16();


#ifdef REGION_TIMING
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;
#endif

    return 0;
}
