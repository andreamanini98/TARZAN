#include "TARZAN/parser/ast.h"
#include "TARZAN/parser/timed_automaton.h"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>


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


int main()
{
    // TODO: clear this mess and make it suitable for testing the parsing of only a timed automaton.
    // maybe override to_string for timedAutomaton struct.

    using boost::spirit::x3::ascii::space;
    using iterator_type = std::string::const_iterator;
    namespace x3 = boost::spirit::x3;


    // TESTING TIMED AUTOMATON

    ast::timedAutomaton ta;

    std::string newstr = readFromFile("ta0.txt");
    std::cout << newstr << std::endl;
    iterator_type iter = newstr.begin();
    iterator_type const end = newstr.end();

    // Make sure iter is pointing to 'c'
    if (iter != end)
    {
        std::cout << "Full parser input starts with: '" << *iter << "'" << std::endl;
    }

    // Crucially, get the rule from the global namespace function
    auto const &automaton_parser_rule = timed_automaton::timedAutomaton(); // Correct way to get the rule

    bool r = x3::phrase_parse(iter, end, automaton_parser_rule, space, ta);

    if (r && iter == end)
    {
        std::cout << "-------------------------\n";
        std::cout << "Parsing succeeded\n";
        std::cout << "ta name: " << ta.name << std::endl;
    } else
    {
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        if (iter != end)
        {
            // TODO: can we print where we are when parsing?
            std::cout << "Failed at: '" << *iter << "' (char code: " << static_cast<int>(*iter) << ")" << std::endl;
            // Print a bit of context
            std::cout << "Context: \"";
            for (auto c_iter = iter; c_iter != end && std::distance(iter, c_iter) < 20; ++c_iter)
            {
                std::cout << *c_iter;
            }
            std::cout << "...\"" << std::endl;
        } else
        {
            std::cout << "Failed at end of input, but not all input consumed." << std::endl;
        }
        std::cout << "-------------------------\n";
    }


    // TESTING ARENA

    ast::timedArena arena;

    std::string tarstr = readFromFile("arena0.txt");
    std::cout << newstr << std::endl;
    iterator_type iter2 = tarstr.begin();
    iterator_type const end2 = tarstr.end();

    // Make sure iter is pointing to 'c'
    if (iter2 != end2)
    {
        std::cout << "Full parser input starts with: '" << *iter2 << "'" << std::endl;
    }

    // Crucially, get the rule from the global namespace function
    auto const &timed_arena_parser_rule = timed_automaton::timedArena(); // Correct way to get the rule

    bool r2 = x3::phrase_parse(iter2, end2, timed_arena_parser_rule, space, arena);

    if (r2 && iter2 == end2)
    {
        std::cout << "-------------------------\n";
        std::cout << "Arena parsing succeeded\n";
        std::cout << "arena name: " << arena.name << std::endl;
    }


    return 0;
}
