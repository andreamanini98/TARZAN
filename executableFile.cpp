#include "TARZAN/parser/ast.h"
#include "TARZAN/parser/timed_automaton.h"
#include "TARZAN/parser/error_handler.h"
#include "TARZAN/parser/config.h"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>


std::string readFromFile(const std::string &relativePath)
{
    // Fixed path starting from the project root
    std::string fullPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/examples/timed-automata-definitions/" + relativePath;
    // Adjust the number of "../" as needed

    std::ifstream file(fullPath);
    if (!file.is_open())
    {
        std::cerr << "Attempted to open: " << fullPath << std::endl;
        throw std::runtime_error("Failed to open file: " + relativePath);
    }

    std::string content{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    file.close();
    return content;
}


// TODO: this parse function is ok, you can enhance this and make it a library function.
// TODO: then, you create another fot the arena, maybe you can reuse some code
timed_automaton::ast::timedArena parse_Arena(std::string const &source)
{
    std::stringstream out;

    using timed_automaton::parser::iterator_type;
    iterator_type iter(source.begin());
    iterator_type const end(source.end());

    // Our AST
    timed_automaton::ast::timedArena ast;

    // Our error handler
    using boost::spirit::x3::with;
    using timed_automaton::parser::error_handler_type;
    using timed_automaton::parser::error_handler_tag;
    // TODO: replace with the path of the file under parsing.
    error_handler_type error_handler(iter, end, out, "/Users"); // Our error handler

    // Our parser
    auto const parser =
            // we pass our error handler to the parser so we can access
            // it later on in our on_error and on_success handlers
            with<error_handler_tag>(std::ref(error_handler))
            [
                timed_automaton::timedArena()
            ];

    // Go forth and parse!
    using boost::spirit::x3::ascii::space;
    bool success = phrase_parse(iter, end, parser, space, ast);

    if (success)
    {
        if (iter != end)
        {
            error_handler(iter, "Error! Expecting end of input here: ");
        } else
            std::cerr << "SUCCESSful parsing" << std::endl;
    } else
        std::cerr << "Wrong parsing" << std::endl;

    return ast;
};


int main()
{
    timed_automaton::ast::timedArena arena = parse_Arena(readFromFile("arena0.txt"));
    std::cout << "\n\n\n PARSED ARENA \n" << std::endl;
    std::cout << arena << std::endl;

    // using boost::spirit::x3::ascii::space;
    // using iterator_type = std::string::const_iterator;
    // namespace x3 = boost::spirit::x3;
    //
    //
    // // TESTING TIMED AUTOMATON
    //
    // timed_automaton::ast::timedAutomaton ta;
    //
    // std::string newstr = readFromFile("ta0.txt");
    // std::cout << newstr << std::endl;
    // iterator_type iter = newstr.begin();
    // iterator_type const end = newstr.end();
    //
    // // Make sure iter is pointing to 'c'
    // if (iter != end)
    // {
    //     std::cout << "Full parser input starts with: '" << *iter << "'" << std::endl;
    // }
    //
    // // Crucially, get the rule from the global namespace function
    // auto const &automaton_parser_rule = timed_automaton::timedAutomaton(); // Correct way to get the rule
    //
    // bool r = x3::phrase_parse(iter, end, automaton_parser_rule, space, ta);
    //
    // if (r && iter == end)
    // {
    //     std::cout << "-------------------------\n";
    //     std::cout << "Parsing succeeded\n";
    //     std::cout << "ta name: " << ta.name << std::endl;
    // } else
    // {
    //     std::cout << "-------------------------\n";
    //     std::cout << "Parsing failed\n";
    //     if (iter != end)
    //     {
    //         // TODO: can we print where we are when parsing?
    //         std::cout << "Failed at: '" << *iter << "' (char code: " << static_cast<int>(*iter) << ")" << std::endl;
    //         // Print a bit of context
    //         std::cout << "Context: \"";
    //         for (auto c_iter = iter; c_iter != end && std::distance(iter, c_iter) < 20; ++c_iter)
    //         {
    //             std::cout << *c_iter;
    //         }
    //         std::cout << "...\"" << std::endl;
    //     } else
    //     {
    //         std::cout << "Failed at end of input, but not all input consumed." << std::endl;
    //     }
    //     std::cout << "-------------------------\n";
    // }

    return 0;
}
