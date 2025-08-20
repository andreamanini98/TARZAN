#include "TARZAN/headers/library.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "TARZAN/parser/ast.h"
#include "TARZAN/parser/timed_automaton.h"
#include "TARZAN/parser/error_handler.h"
#include "TARZAN/parser/config.h"
#include "TARZAN/utilities/file_utilities.h"


timed_automaton::ast::timedArena parseTimedArena(std::string const &path)
{
    std::stringstream out;
    const std::string source = readFromFile(path);

    using timed_automaton::parser::iterator_type;
    iterator_type iter(source.begin());
    iterator_type const end(source.end());

    // Our Timed Arena AST.
    timed_automaton::ast::timedArena arena;

    // Our error handler.
    using boost::spirit::x3::with;
    using timed_automaton::parser::error_handler_type;
    using timed_automaton::parser::error_handler_tag;
    error_handler_type error_handler(iter, end, out, path);

    // Our parser.
    // We pass our error handler to the parser so we can access it later on in our on_error and on_success handlers.
    auto const parser = with<error_handler_tag>(std::ref(error_handler))[timed_automaton::timedArena()];

    // Now we parse.
    using boost::spirit::x3::ascii::space;
    // ReSharper disable once CppTooWideScope
    bool success = phrase_parse(iter, end, parser, space, arena);

    if (success)
    {
        if (iter != end)
            error_handler(iter, "Error! Expecting end of input here: ");
        else
            std::cerr << "SUCCESSFUL parsing" << std::endl;
    } else
        std::cerr << "Wrong parsing" << std::endl;

    return arena;
}


timed_automaton::ast::timedAutomaton parseTimedAutomaton(std::string const &path)
{
    std::stringstream out;
    const std::string source = readFromFile(path);

    using timed_automaton::parser::iterator_type;
    iterator_type iter(source.begin());
    iterator_type const end(source.end());

    // Our Timed Automaton AST.
    timed_automaton::ast::timedAutomaton automaton;

    // Our error handler.
    using boost::spirit::x3::with;
    using timed_automaton::parser::error_handler_type;
    using timed_automaton::parser::error_handler_tag;
    error_handler_type error_handler(iter, end, out, path);

    // Our parser.
    // We pass our error handler to the parser so we can access it later on in our on_error and on_success handlers.
    auto const parser = with<error_handler_tag>(std::ref(error_handler))[timed_automaton::timedAutomaton()];

    // Now we parse.
    using boost::spirit::x3::ascii::space;
    // ReSharper disable once CppTooWideScope
    bool success = phrase_parse(iter, end, parser, space, automaton);

    if (success)
    {
        if (iter != end)
            error_handler(iter, "Error! Expecting end of input here: ");
        else
            std::cerr << "SUCCESSFUL parsing" << std::endl;
    } else
        std::cerr << "Wrong parsing" << std::endl;

    return automaton;
}
