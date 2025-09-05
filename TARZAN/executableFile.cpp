#include <string>

#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "absl/container/flat_hash_set.h"

// #define REGION_PRINT_DEBUG


region::Region getRegion(int numSteps)
{
    region::Region reg(3);

    const auto newH = static_cast<int *>(malloc(sizeof(int) * 3));
    newH[0] = 0;
    newH[1] = 0;
    newH[2] = 1;

    reg.set_h(newH);

    boost::dynamic_bitset<> xm1(3);
    boost::dynamic_bitset<> x0(3);
    boost::dynamic_bitset<> x1(3);

    xm1.set(0, true);
    x0.set(2, true);
    x1.set(1, true);

    const std::deque Xm1{ xm1 };
    const std::deque X1{ x1 };

    reg.set_unbounded(Xm1);
    reg.set_x0(x0);
    reg.set_bounded(X1);

    region::Region oldSuccessor = reg;
    for (int i = 0; i < numSteps; i++)
    {
        region::Region successor = oldSuccessor.getImmediateDelaySuccessor(1);
        oldSuccessor = successor;
    }

    return oldSuccessor;
}


void testParsing()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";

    const std::string automatonFileName = "light_switch.txt";
    //const std::string arenaFileName = "arena0.txt";

    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    //timed_automaton::ast::timedArena arena = parseTimedArena(path + arenaFileName);

    std::cout << "\n\n\n\n\n";
    std::cout << "Parsed automaton: " << automaton << std::endl;

    //std::cout << "\n\n\n\n\n";
    //std::cout << "Parsed arena: " << arena << std::endl;
}


void testImmediateDelaySuccessor()
{
    region::Region oldSuccessor = getRegion(0);
    for (int i = 0; i < 7; i++)
    {
        region::Region successor = oldSuccessor.getImmediateDelaySuccessor(1);
        std::cout << "Successor " << (i + 1) << ":\n" << successor.toString() << std::endl;
        oldSuccessor = successor;
    }
}


void testImmediateDelayPredecessors(const int totSteps, const int maxConst)
{
    region::Region oldSuccessor = getRegion(0);

    for (int i = 0; i < totSteps; i++)
    {
#ifdef REGION_PRINT_DEBUG
        std::cout << oldSuccessor.toString() << std::endl;
#endif
        const region::Region successor = oldSuccessor.getImmediateDelaySuccessor(maxConst);
        oldSuccessor = successor;
    }

    std::cout << oldSuccessor.toString() << std::endl;

    std::vector<region::Region> oldPred;
    std::vector<region::Region> newPred;
    oldPred.push_back(oldSuccessor);

    for (int i = 0; i < totSteps; i++)
    {
        for (const region::Region &p: oldPred)
        {
            std::vector<region::Region> predecessors = p.getImmediateDelayPredecessors();
#ifdef REGION_PRINT_DEBUG
            if (!predecessors.empty())
            {
                std::cout << "Predecessors!\n";
                for (const region::Region &r: predecessors)
                {
                    std::cout << "Predecessor " << (i + 1) << ":\n";
                    std::cout << r.toString() << std::endl;
                }
            }
#endif
            newPred.insert(newPred.end(), predecessors.begin(), predecessors.end());
        }

        oldPred = newPred;
        newPred.clear();
    }
}


void testLocationMapping()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    const std::string automatonFileName = "ta0.txt";

    const timed_automaton::ast::timedAutomaton TA = parseTimedAutomaton(path + automatonFileName);

    std::cout << TA << std::endl;

    // ReSharper disable once CppTooWideScopeInitStatement
    std::unordered_map<std::string, int> map = TA.mapLocationsToInt();

    for (const auto &[key, value]: map)
        std::cout << key << ": " << value << std::endl;

    std::cout << "\nNow printing the transitions:" << std::endl;
    const std::vector<std::vector<timed_automaton::ast::transition>> outTransitions = TA.getOutTransitions(map);

    std::cout << outTransitions.size() << std::endl;

    for (size_t i = 0; i < outTransitions.size(); ++i)
    {
        std::cout << "Index " << i << ":" << std::endl;
        for (const auto &transition: outTransitions[i])
            std::cout << "  " << transition << std::endl;
        std::cout << std::endl;
    }
}


void testClockValuation()
{
    const region::Region reg = getRegion(4);

    std::cout << "We now test the clock valuation of: " << reg.toString() << std::endl;

    const std::vector<std::pair<int, bool>> clockValuation = reg.getClockValuation();
    std::ostringstream oss;
    oss << "Clock Valuation: [";
    for (size_t i = 0; i < clockValuation.size(); i++)
    {
        if (i > 0) oss << ", ";
        oss << "clock" << i << ": (" << clockValuation[i].first
                << ", " << (clockValuation[i].second ? "has_fraction" : "no_fraction") << ")";
    }
    oss << "]";
    std::cout << oss.str() << std::endl;
}


void testClockIndices()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";

    const std::string automatonFileName = "ta0.txt";

    // ReSharper disable once CppTooWideScopeInitStatement
    timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    for (const auto &[fst, snd]: automaton.getClocksIndices())
    {
        std::cout << fst << ", " << snd << std::endl;
    }
}


void testTransitionIsSatisfiable()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    const std::string automatonFileName = "ta0.txt";

    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    std::cout << "\n\n\nThe transitions of the automaton are:" << std::endl;
    for (const auto &tr: automaton.transitions)
        std::cout << tr << std::endl;

    std::vector<std::pair<int, bool>> clockValuation{};
    clockValuation.emplace_back(2, true);
    clockValuation.emplace_back(5, false);
    clockValuation.emplace_back(5, true);

    std::unordered_map<std::string, int> clocksIndices{};
    clocksIndices.emplace("x", 0);
    clocksIndices.emplace("y", 1);
    clocksIndices.emplace("z", 2);

    std::cout << "Now trying to see if the transitions are satisfied:" << std::endl;
    for (const auto &tr: automaton.transitions)
        std::cout << tr << ": \n" << tr.isGuardSatisfied(clockValuation, clocksIndices) << std::endl;
}


void testImmediateDiscreteSuccessors()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    const std::string automatonFileName = "ta0.txt";

    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    std::cout << "\n\n\n";

    region::Region reg(3);

    const auto newH = static_cast<int *>(malloc(sizeof(int) * 3));
    newH[0] = 0;
    newH[1] = 0;
    newH[2] = 0;
    reg.set_h(newH);

    boost::dynamic_bitset<> x0(3);
    x0.set(0, true);
    x0.set(1, true);
    x0.set(2, true);
    reg.set_x0(x0);

    std::unordered_map<std::string, int> locationsIntMap = automaton.mapLocationsToInt();

    // std::cout << "Location to Integer Mapping:\n";
    // for (const auto& [location, index] : locationsIntMap) {
    //     std::cout << location << " -> " << index << std::endl;
    // }

    reg.set_q(locationsIntMap.at("q0"));

    std::vector<std::vector<transition>> outTransitions = automaton.getOutTransitions(locationsIntMap);

    // std::cout << "Outgoing Transitions Map:\n";
    // for (size_t i = 0; i < outTransitionsMap.size(); ++i) {
    //     std::cout << "Location Index " << i << " (" << outTransitionsMap[i].size() << " transitions):\n";
    //     for (const auto& transition : outTransitionsMap[i]) {
    //         std::cout << "  " << transition << std::endl;
    //     }
    //     std::cout << std::endl;
    // }

    std::unordered_map<std::string, int> clockIndices = automaton.getClocksIndices();

    // std::cout << "Clock to Index Mapping:\n";
    // for (const auto& [clockName, index] : clockIndices) {
    //     std::cout << clockName << " -> " << index << std::endl;
    // }

    std::cout << "Initial region: " << reg.toString() << std::endl;

    region::Region oldSuccessor = reg;
    bool computedDiscreteSuccessor = false;

    while (!computedDiscreteSuccessor)
    {
        std::vector<region::Region> discreteSuccessors = oldSuccessor.getImmediateDiscreteSuccessors(
            outTransitions[reg.getLocation()], clockIndices, locationsIntMap
        );

        if (discreteSuccessors.empty())
            std::cout << "Discrete successors are empty!" << std::endl;
        else
        {
            std::cout << "Discrete successors found!" << std::endl;
            std::cout << "Discrete Successors:\n";
            std::cout << std::string(50, '-') << std::endl;

            int count = 1;
            for (const auto &region: discreteSuccessors)
            {
                oldSuccessor = region;
                std::cout << "region::Region #" << count++ << ":\n";
                std::cout << region.toString();
                std::cout << std::string(30, '-') << std::endl;
            }

            computedDiscreteSuccessor = true;
            break;
        }

        region::Region successor = oldSuccessor.getImmediateDelaySuccessor(10);
        std::cout << "Delay successor :\n" << successor.toString() << std::endl;
        oldSuccessor = successor;
    }

    reg = oldSuccessor;
    std::cout << "New initial region: " << reg.toString() << std::endl;

    oldSuccessor = reg;
    computedDiscreteSuccessor = false;

    while (!computedDiscreteSuccessor)
    {
        std::vector<region::Region> discreteSuccessors = oldSuccessor.getImmediateDiscreteSuccessors(
            outTransitions[reg.getLocation()], clockIndices, locationsIntMap
        );

        if (discreteSuccessors.empty())
            std::cout << "Discrete successors are empty!" << std::endl;
        else
        {
            std::cout << "Discrete successors found!" << std::endl;
            std::cout << "Discrete Successors:\n";
            std::cout << std::string(50, '-') << std::endl;

            int count = 1;
            for (const auto &region: discreteSuccessors)
            {
                oldSuccessor = region;
                std::cout << "region::Region #" << count++ << ":\n";
                std::cout << region.toString();
                std::cout << std::string(30, '-') << std::endl;
            }

            computedDiscreteSuccessor = true;
            break;
        }

        region::Region successor = oldSuccessor.getImmediateDelaySuccessor(10);
        std::cout << "Delay successor :\n" << successor.toString() << std::endl;
        oldSuccessor = successor;
    }
}


void testIsInitial()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    const std::string automatonFileName = "ta0.txt";

    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    std::cout << "\n\n\n";

    std::unordered_map<std::string, int> locationsIntMap = automaton.mapLocationsToInt();

    std::cout << "Location to Integer Mapping:\n";
    for (const auto &[location, index]: locationsIntMap)
    {
        std::cout << location << " -> " << index << std::endl;
    }

    std::cout << "initial locations are: " << std::endl;
    for (const std::vector<int> initials = automaton.getInitialLocations(locationsIntMap); const int loc: initials)
        std::cout << loc << std::endl;
}


void testHash()
{
    region::Region r = getRegion(3);

    std::cout << "Region: " << r.toString() << std::endl;

    absl::flat_hash_set<region::Region, region::RegionHash> regions{};

    regions.insert(r);

    std::cout << "is r in regions? " << regions.contains(r) << std::endl;

    region::Region r1 = getRegion(3);

    regions.insert(r1);

    region::Region r2 = getRegion(3);

    std::cout << "is r in regions? " << regions.contains(r2) << std::endl;

    region::Region r3 = getRegion(6);

    std::cout << "is r in regions? " << regions.contains(r3) << std::endl;
}


void testRTS()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    // const std::string automatonFileName = "light_switch.txt";

    const std::string automatonFileName = "test_flower_small.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    const region::RTS regionTransitionSystem(automaton);

    //std::cout << "Location to Integer Mapping:\n";
    //for (const auto& [location, index] : automaton.mapLocationsToInt()) {
    //    std::cout << location << " -> " << index << std::endl;
    //}

    std::cout << "\n\n\n";

    std::cout << "Computed the following regions:" << std::endl;

    const std::vector<region::Region> rts = regionTransitionSystem.buildRegionGraphForeword();

    //for (const std::vector<region::Region> rts = regionTransitionSystem.buildRegionGraphForeword(); const auto &region: rts)
    //    std::cout << region.toString() << std::endl;
}


int main()
{
    //std::cout << "Tick period: " << static_cast<double>(std::chrono::high_resolution_clock::period::num) / std::chrono::high_resolution_clock::period::den << " seconds\n";
    //const auto start = std::chrono::high_resolution_clock::now();

    testRTS();

#ifdef _OPENMP
    std::cout << "Compiled with OpenMP!!" << std::endl;
#else
    std::cout << "OpenMP not available!" << std::endl;
#endif

    //const auto end = std::chrono::high_resolution_clock::now();
    //const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    //std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;

    return 0;
}
