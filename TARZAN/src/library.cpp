#include "TARZAN/headers/library.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

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


std::vector<timed_automaton::ast::timedAutomaton> parseTimedAutomataFromFolder(std::string const &folderPath)
{
    std::vector<timed_automaton::ast::timedAutomaton> automata;

    try
    {
        // Iterate through all files in the folder.
        for (const auto &entry: std::filesystem::directory_iterator(folderPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".txt")
            {
                std::string filePath = entry.path().string();
                std::cerr << "Parsing automaton from file: " << filePath << std::endl;

                // Parse the automaton using the existing function.
                timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(filePath);
                automata.push_back(automaton);
            }
        }
    } catch (const std::filesystem::filesystem_error &ex)
    {
        std::cerr << "Filesystem error: " << ex.what() << std::endl;
    }

    return automata;
}


std::vector<timed_automaton::ast::timedArena> parseTimedArenasFromFolder(std::string const &folderPath)
{
    std::vector<timed_automaton::ast::timedArena> arenas;

    try
    {
        // Iterate through all files in the folder.
        for (const auto &entry: std::filesystem::directory_iterator(folderPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".txt")
            {
                std::string filePath = entry.path().string();
                std::cerr << "Parsing arena from file: " << filePath << std::endl;

                // Parse the arena using the existing function.
                timed_automaton::ast::timedArena arena = parseTimedArena(filePath);
                arenas.push_back(arena);
            }
        }
    } catch (const std::filesystem::filesystem_error &ex)
    {
        std::cerr << "Filesystem error: " << ex.what() << std::endl;
    }

    return arenas;
}
