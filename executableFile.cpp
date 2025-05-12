#include "TARZAN/parser/ast.h"
#include "TARZAN/parser/timed_automaton.h"

#include <boost/fusion/include/io.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include <fstream>
#include <string>
#include <iostream>

std::string readFromFile(const std::string& relativePath) {
    // Fixed path starting from the project root
    std::string fullPath = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/timed-automata-definitions/" + relativePath; // Adjust the number of "../" as needed

    std::ifstream file(fullPath);
    if (!file.is_open()) {
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
        std::cout << boost::fusion::tuple_open('[');
        std::cout << boost::fusion::tuple_close(']');
        std::cout << boost::fusion::tuple_delimiter(", "[0]);

        std::cout << "-------------------------\n";
        std::cout << "Parsing succeeded\n";
        std::cout << "ta name: " << ta.name << std::endl;
        if (!ta.clocks.empty())
        {
            // Check if clocks is not empty before accessing
            std::cout << "Got clocks: ";
            for (size_t i = 0; i < ta.clocks.size(); ++i)
            {
                std::cout << ta.clocks[i] << (i == ta.clocks.size() - 1 ? "" : ", ");
            }
            std::cout << std::endl;
        } else
        {
            std::cout << "got: (no clocks parsed)" << std::endl;
        }
        if (!ta.actions.empty())
        {
            // Check if clocks is not empty before accessing
            std::cout << "Got actions: ";
            for (size_t i = 0; i < ta.actions.size(); ++i)
            {
                std::cout << ta.actions[i] << (i == ta.actions.size() - 1 ? "" : ", ");
            }
            std::cout << std::endl;
        } else
        {
            std::cout << "got: (no actions parsed)" << std::endl;
        }
        if (!ta.locations.empty()) {
            std::cout << "Got locations: " << std::endl;
            for (const auto& [locationName, isInitial] : ta.locations) {
                std::cout << "  " << locationName;
                if (isInitial) {
                    std::cout << " (initial: " << (isInitial.get() ? "true" : "false") << ")";
                } else {
                    std::cout << " (initial not specified)";
                }
                std::cout << std::endl;
            }
        } else {
            std::cout << "got: (no locations parsed)" << std::endl;
        }

        if (!ta.transitions.empty())
        {
            std::cout << "Got transitions: " << std::endl;
            for (size_t i = 0; i < ta.transitions.size(); ++i)
            {
                const auto &trans = ta.transitions[i];
                std::cout << "  Transition " << (i + 1) << ":" << std::endl;
                std::cout << "    From: " << trans.startingLocation << std::endl;
                std::cout << "    Action: " << trans.action << std::endl;

                // Print clock guards
                std::cout << "    Clock Guards: ";
                if (trans.clockGuard.empty())
                {
                    std::cout << "(none)";
                } else
                {
                    std::cout << std::endl;
                    for (size_t j = 0; j < trans.clockGuard.size(); ++j)
                    {
                        std::cout << "      " << trans.clockGuard[j].clock << " "
                                << trans.clockGuard[j].guardOperator << " "
                                << trans.clockGuard[j].comparingConstant;
                        if (j < trans.clockGuard.size() - 1)
                        {
                            std::cout << ",";
                        }
                        std::cout << std::endl;
                    }
                }

                // Print clocks to reset
                std::cout << "    Clocks to Reset: ";
                if (trans.clocksToReset.empty())
                {
                    std::cout << "(none)";
                } else
                {
                    for (size_t j = 0; j < trans.clocksToReset.size(); ++j)
                    {
                        std::cout << trans.clocksToReset[j];
                        if (j < trans.clocksToReset.size() - 1)
                        {
                            std::cout << ", ";
                        }
                    }
                }
                std::cout << std::endl;

                std::cout << "    To: " << trans.targetLocation << std::endl;
                if (i < ta.transitions.size() - 1)
                {
                    std::cout << std::endl;
                }
            }
        } else
        {
            std::cout << "got: (no transitions parsed)" << std::endl;
        }

        std::cout << "\n-------------------------\n";
    } else
    {
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        if (iter != end)
        {
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



    return 0;
}
