#include <string>

#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/Region.h"

// #define REGION_PRINT_REBUG


void testParsing()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";

    const std::string automatonFileName = "ta0.txt";
    const std::string arenaFileName = "arena0.txt";

    timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    timed_automaton::ast::timedArena arena = parseTimedArena(path + arenaFileName);

    std::cout << "\n\n\n\n\n";
    std::cout << "Parsed automaton: " << automaton << std::endl;

    std::cout << "\n\n\n\n\n";
    std::cout << "Parsed arena: " << arena << std::endl;
}


void testImmediateDelaySuccessor()
{
    Region reg(3);

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

    std::cout << "REG:\n" << reg.toString() << std::endl;

    Region oldSuccessor = reg;
    for (int i = 0; i < 7; i++)
    {
        Region successor = oldSuccessor.getImmediateDelaySuccessor(1);
        std::cout << "Successor " << (i + 1) << ":\n" << successor.toString() << std::endl;
        oldSuccessor = successor;
    }
}


void testImmediateDelayPredecessors(int totSteps, int maxConst)
{
    Region reg(3);

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

    Region oldSuccessor = reg;
    for (int i = 0; i < totSteps; i++)
    {
#ifdef REGION_PRINT_REBUG
        std::cout << oldSuccessor.toString() << std::endl;
#endif
        Region successor = oldSuccessor.getImmediateDelaySuccessor(maxConst);
        oldSuccessor = successor;
    }

    std::cout << oldSuccessor.toString() << std::endl;

    std::vector<Region> oldPred;
    std::vector<Region> newPred;
    oldPred.push_back(oldSuccessor);

    for (int i = 0; i < totSteps; i++)
    {
        for (const Region &p: oldPred)
        {
            std::vector<Region> predecessors = p.getImmediateDelayPredecessors();
#ifdef REGION_PRINT_REBUG
            if (!predecessors.empty())
            {
                std::cout << "Predecessors!\n";
                for (const Region &r: predecessors)
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
    Region reg(3);

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

    Region oldSuccessor = reg;
    for (int i = 0; i < 4; i++)
    {
        Region successor = oldSuccessor.getImmediateDelaySuccessor(3);
        oldSuccessor = successor;
    }

    std::cout << "We now test the clock valuation of: " << oldSuccessor.toString() << std::endl;

    std::vector<std::pair<int, bool>> clockValuation = oldSuccessor.getClockValuation();
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


int main()
{
    // std::cout << "Tick period: " << static_cast<double>(std::chrono::high_resolution_clock::period::num) / std::chrono::high_resolution_clock::period::den << " seconds\n";
    // const auto start = std::chrono::high_resolution_clock::now();
    // const auto end = std::chrono::high_resolution_clock::now();
    // const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;

    testClockValuation();

    return 0;
}
