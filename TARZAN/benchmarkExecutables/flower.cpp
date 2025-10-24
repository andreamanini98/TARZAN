#include "TARZAN/parser/ast.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/utilities/file_utilities.h"


inline void testFlower()
{
    // TODO: fare in modo che da linea di comando si possano passare al main il path del benchmark e il nome del file del benchmark.
    //       Il nome del benchmark dovrà corrispondere a quello del file.
    //       Uno script bash (crea una cartella scripts) orchestrerà l'esecuzione in base alla cartella del benchmark e alle sue eventuali sottocartelle.
    //       In questo modo puoi tenere early exit.
    //       Va creato un file .cpp er ogni benchmark.

    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/benchmarks/flower/liana/flower_10/";
    const std::string automatonFileName = "flower_10.txt";
    const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);

    benchmark::benchmarkName = "b1";

    const region::RTS regionTransitionSystem(automaton);

    const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);
}


int main()
{
    testFlower();

    return 0;
}
