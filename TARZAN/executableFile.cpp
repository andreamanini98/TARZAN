#include <string>

#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/utilities/file_utilities.h"


int main()
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

    return 0;
}
