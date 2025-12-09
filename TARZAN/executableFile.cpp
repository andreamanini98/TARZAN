#include <string>

#include "regions/networkOfTA/RTSNetwork.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/utilities/partition_utilities.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/headers/library.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"

#include <algorithm>
#include "TARZAN/testing/successorsAndPredecessorsTesting.h"
#include "TARZAN/parser/timed_automaton_def.h"
#include "TARZAN/parser/config.h"


inline void testArenaParsing()
{
    const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/games/";
    const std::string arenaName = "arena0.txt";
    const timed_automaton::ast::timedArena arena = TARZAN::parseTimedArena(path + arenaName);

    std::cout << arena.to_string() << std::endl;

    const auto &locToIntMap = arena.mapLocationsToInt();

    std::cout << "Locations to int map: " << locToIntMap.size() << std::endl;
    for (const auto &locToInt: locToIntMap)
        std::cout << locToInt.first << " - " << locToInt.second << std::endl;

    const auto &locToPlayerMap = arena.mapLocationsToPlayers(locToIntMap);

    std::cout << "Locations to player map: " << locToPlayerMap.size() << std::endl;
    for (const auto &locToPlayer: locToPlayerMap)
        std::cout << locToPlayer.first << " - " << locToPlayer.second << std::endl;

    std::cout << "Clock indices: " << std::endl;
    for (const auto &clockIndices: arena.getClocksIndices())
        std::cout << clockIndices.first << " - " << clockIndices.second << std::endl;

    const region::RTS rts(arena);

    std::cout << rts.to_string() << std::endl;
}


inline void testCLTLocFormulaParsing()
{
    // Parse a CLTLoc formula from a file.
    std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/CLTLoc_formulae/formula0.txt";
    std::stringstream out;
    const std::string source = readFromFile(path);

    using parser::iterator_type;
    iterator_type iter(source.begin());
    iterator_type const end(source.end());

    cltloc::ast::generalCLTLocFormula phi;

    // Our error handler.
    using boost::spirit::x3::with;
    using parser::error_handler_type;
    using parser::error_handler_tag;
    error_handler_type error_handler(iter, end, out, path);

    // Our parser.
    // We pass our error handler to the parser so we can access it later on in our on_error and on_success handlers.
    auto const parser = with<error_handler_tag>(std::ref(error_handler))[cltloc::generalCLTLocFormula()];

    // Now we parse.
    using boost::spirit::x3::ascii::space;
    // ReSharper disable once CppTooWideScope
    bool success = phrase_parse(iter, end, parser, space, phi);

    if (success)
    {
        if (iter != end)
            error_handler(iter, "Error! Expecting end of input here: ");
        else
            std::cout << "SUCCESSFUL parsing" << std::endl;
    } else
        std::cerr << "Wrong parsing" << std::endl;

    std::cout << phi.to_string() << std::endl;
}


int main()
{
    testCLTLocFormulaParsing();

    return 0;
}
