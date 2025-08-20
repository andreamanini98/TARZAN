#include "parser/ast.h"
#include "parser/timed_automaton.h"
#include "parser/error_handler.h"
#include "parser/config.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

// TODO: move this parsing function into the library.h and library.cpp files.


/** This is a doxygen comment
*
* Long comment
* ~~~.cpp
* int x = 5; // comment
* ~~~
* @param absolutePath: the absolute path in which the file to read is located.
* @param fileName the name of the file to read.
* @returns a string of the read file.
* @note test note
* @attention attention test
* @warning warning test
**/
std::string readFromFile(const std::string &absolutePath, const std::string &fileName)
{
    const std::string fullPath = absolutePath + fileName;

    std::ifstream file(fullPath);
    if (!file.is_open())
    {
        std::cerr << "Attempted to open: " << fullPath << std::endl;
        throw std::runtime_error("Failed to open file: " + fileName);
    }

    std::string content{ std::istreambuf_iterator(file), std::istreambuf_iterator<char>() };

    file.close();
    return content;
}


// TODO: make this return the arena (or the automaton).
std::string parseTimedArena(std::string const &source, std::string const &absolutePath)
{
    std::stringstream out;

    using timed_automaton::parser::iterator_type;
    iterator_type iter(source.begin());
    iterator_type const end(source.end());

    // Our AST.
    timed_automaton::ast::timedArena ast;

    // Our error handler.
    using boost::spirit::x3::with;
    using timed_automaton::parser::error_handler_type;
    using timed_automaton::parser::error_handler_tag;
    error_handler_type error_handler(iter, end, out, absolutePath);

    // Our parser.
    // We pass our error handler to the parser so we can access it later on in our on_error and on_success handlers.
    auto const parser = with<error_handler_tag>(std::ref(error_handler))[timed_automaton::timedArena()];

    // Now we parse.
    using boost::spirit::x3::ascii::space;
    // ReSharper disable once CppTooWideScope
    bool success = phrase_parse(iter, end, parser, space, ast);

    if (success)
    {
        if (iter != end)
            error_handler(iter, "Error! Expecting end of input here: ");
        else
            std::cerr << "SUCCESSFUL parsing" << std::endl;
    } else
        std::cerr << "Wrong parsing" << std::endl;

    return out.str();
}


int main()
{
    const std::string absolutePath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
    const std::string fileName = "arena0.txt";
    std::cout << parseTimedArena(readFromFile(absolutePath, fileName), absolutePath + fileName) << std::endl;

    return 0;
}
