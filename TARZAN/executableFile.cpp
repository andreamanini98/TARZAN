#include <string>

#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/regions/Region.h"


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


int main()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    const std::string automatonFileName = "ta0.txt";
    const timed_automaton::ast::timedAutomaton TA = parseTimedAutomaton(path + automatonFileName);

    const int maxConstant = TA.getMaxConstant();

    const Region reg(TA.clocks.size());
    std::cout << "reg:\n";
    std::cout << reg.toString() << std::endl;

    return 0;
}
