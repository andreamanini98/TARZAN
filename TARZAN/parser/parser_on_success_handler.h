#ifndef PARSER_ON_SUCCESS_HANDLERS_H
#define PARSER_ON_SUCCESS_HANDLERS_H

#include <iostream>
#include <string>

namespace timed_automaton::parser {
    struct loc_pair_class {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &first, Iterator const &last, T &ast, Context const &)
        {
            std::string input(first, last);
            std::cout << "[PARSER] Matched Timed Automaton location: " << ast.first;
            if (ast.second)
                std::cout << " <initial: " << (ast.second.get() ? "true" : "false") << ">";
            std::cout << std::endl;
        }
    };

    struct loc_map_class {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &, Iterator const &, T &ast, Context const &)
        {
            std::cout << "[PARSER] Matched Timed Automaton locations section with "
                    << ast.size() << " locations:" << std::endl;
            for (const auto &[name, initial]: ast)
            {
                std::cout << "  - " << name;
                if (initial)
                    std::cout << " <initial: " << (initial.get() ? "true" : "false") << ">";
                std::cout << std::endl;
            }
        }
    };

    struct arena_loc_class {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &first, Iterator const &last, T &ast, Context const &)
        {
            std::string input(first, last);
            std::cout << "[PARSER] Matched Timed Arena location attributes: <player: " << ast.first;
            if (ast.second)
                std::cout << ", initial: " << (ast.second.get() ? "true" : "false");
            std::cout << ">" << std::endl;
        }
    };

    struct arena_loc_pair_class {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &, Iterator const &, T &ast, Context const &)
        {
            std::cout << "[PARSER] Matched Timed Arena location: " << ast.first << " <player: " << ast.second.first;
            if (ast.second.second)
                std::cout << ", initial: " << (ast.second.second.get() ? "true" : "false");
            std::cout << ">" << std::endl;
        }
    };

    struct arena_loc_map_class {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &, Iterator const &, T &ast, Context const &)
        {
            std::cout << "[PARSER] Matched Timed Arena locations section with "
                    << ast.size() << " locations:" << std::endl;
            for (const auto &[name, loc]: ast)
            {
                std::cout << "  - " << name << " <player: " << loc.first;
                if (loc.second)
                    std::cout << ", initial: " << (loc.second.get() ? "true" : "false");
                std::cout << ">" << std::endl;
            }
        }
    };

    struct guard_class {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &first, Iterator const &last, T &ast, Context const &)
        {
            std::string input(first, last);
            std::cout << "[PARSER] Matched guard: ("
                    << ast.clock << ", " << ast.guardOperator << ", " << ast.comparingConstant << ")" << std::endl;
        }
    };

    struct transition_class {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &, Iterator const &, T &ast, Context const &)
        {
            std::cout << "[PARSER] Matched transition:" << std::endl;
            std::cout << "  - From: " << ast.startingLocation << std::endl;
            std::cout << "  - Action: " << ast.action << std::endl;

            std::cout << "  - Clock Guards: ";
            if (ast.clockGuard.empty())
                std::cout << "(none)" << std::endl;
            else
            {
                std::cout << "[" << std::endl;
                for (const auto &guard: ast.clockGuard)
                {
                    std::cout << "    (" << guard.clock << ", " << guard.guardOperator << ", "
                            << guard.comparingConstant << ")" << std::endl;
                }
                std::cout << "  ]" << std::endl;
            }

            std::cout << "  - Clocks to Reset: ";
            if (ast.clocksToReset.empty())
                std::cout << "(none)" << std::endl;
            else
            {
                std::cout << "[";
                for (size_t i = 0; i < ast.clocksToReset.size(); ++i)
                {
                    std::cout << ast.clocksToReset[i];
                    if (i < ast.clocksToReset.size() - 1)
                        std::cout << ", ";
                }
                std::cout << "]" << std::endl;
            }

            std::cout << "  - To: " << ast.targetLocation << std::endl;
        }
    };

    struct timedAutomaton_class {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &, Iterator const &, T &ast, Context const &)
        {
            std::cout << "[PARSER] Matched Timed Automaton:" << std::endl;
            std::cout << "╔══════════════════════════════════════════════════" << std::endl;
            std::cout << "║ Name: " << ast.name << std::endl;

            // Print clocks
            std::cout << "║ Clocks: ";
            if (ast.clocks.empty())
                std::cout << "(none)" << std::endl;
            else
            {
                for (size_t i = 0; i < ast.clocks.size(); ++i)
                {
                    std::cout << ast.clocks[i];
                    if (i < ast.clocks.size() - 1)
                        std::cout << ", ";
                }
                std::cout << std::endl;
            }

            // Print actions
            std::cout << "║ Actions: ";
            if (ast.actions.empty())
                std::cout << "(none)" << std::endl;
            else
            {
                for (size_t i = 0; i < ast.actions.size(); ++i)
                {
                    std::cout << ast.actions[i];
                    if (i < ast.actions.size() - 1)
                        std::cout << ", ";
                }
                std::cout << std::endl;
            }

            // Print locations
            std::cout << "║ Locations:" << std::endl;
            if (ast.locations.empty())
                std::cout << "║   (none)" << std::endl;
            else
            {
                for (const auto &[name, initial]: ast.locations)
                {
                    std::cout << "║   - " << name;
                    if (initial)
                        std::cout << " <initial: " << (initial.get() ? "true" : "false") << ">";
                    std::cout << std::endl;
                }
            }

            // Print transitions
            std::cout << "║ Transitions:" << std::endl;
            if (ast.transitions.empty())
                std::cout << "║   (none)" << std::endl;
            else
            {
                for (size_t i = 0; i < ast.transitions.size(); ++i)
                {
                    const auto &trans = ast.transitions[i];
                    std::cout << "║   " << (i + 1) << ". " << trans.startingLocation << " --("
                            << trans.action << ")--> " << trans.targetLocation << std::endl;

                    // Print guards
                    std::cout << "║      Guards: ";
                    if (trans.clockGuard.empty())
                        std::cout << "(none)" << std::endl;
                    else
                    {
                        std::cout << std::endl;
                        for (const auto &guard: trans.clockGuard)
                        {
                            std::cout << "║        " << guard.clock << " " << guard.guardOperator
                                    << " " << guard.comparingConstant << std::endl;
                        }
                    }

                    // Print resets
                    std::cout << "║      Resets: ";
                    if (trans.clocksToReset.empty())
                        std::cout << "(none)" << std::endl;
                    else
                    {
                        for (size_t j = 0; j < trans.clocksToReset.size(); ++j)
                        {
                            std::cout << trans.clocksToReset[j];
                            if (j < trans.clocksToReset.size() - 1)
                                std::cout << ", ";
                        }
                        std::cout << std::endl;
                    }
                }
            }
            std::cout << "╚══════════════════════════════════════════════════" << std::endl;
        }
    };

    struct timedArena_class {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &, Iterator const &, T &ast, Context const &)
        {
            std::cout << "[PARSER] Matched Timed Arena:" << std::endl;
            std::cout << "╔══════════════════════════════════════════════════" << std::endl;
            std::cout << "║ Name: " << ast.name << std::endl;

            // Print clocks
            std::cout << "║ Clocks: ";
            if (ast.clocks.empty())
                std::cout << "(none)" << std::endl;
            else
            {
                for (size_t i = 0; i < ast.clocks.size(); ++i)
                {
                    std::cout << ast.clocks[i];
                    if (i < ast.clocks.size() - 1)
                        std::cout << ", ";
                }
                std::cout << std::endl;
            }

            // Print actions
            std::cout << "║ Actions: ";
            if (ast.actions.empty())
                std::cout << "(none)" << std::endl;
            else
            {
                for (size_t i = 0; i < ast.actions.size(); ++i)
                {
                    std::cout << ast.actions[i];
                    if (i < ast.actions.size() - 1)
                        std::cout << ", ";
                }
                std::cout << std::endl;
            }

            // Print locations
            std::cout << "║ Locations:" << std::endl;
            if (ast.locations.empty())
                std::cout << "║   (none)" << std::endl;
            else
            {
                for (const auto &[name, loc]: ast.locations)
                {
                    std::cout << "║   - " << name << " <player: " << loc.first;
                    if (loc.second)
                        std::cout << ", initial: " << (loc.second.get() ? "true" : "false");
                    std::cout << ">" << std::endl;
                }
            }

            // Print transitions
            std::cout << "║ Transitions:" << std::endl;
            if (ast.transitions.empty())
                std::cout << "║   (none)" << std::endl;
            else
            {
                for (size_t i = 0; i < ast.transitions.size(); ++i)
                {
                    const auto &trans = ast.transitions[i];
                    std::cout << "║   " << (i + 1) << ". " << trans.startingLocation << " --("
                            << trans.action << ")--> " << trans.targetLocation << std::endl;

                    // Print guards
                    std::cout << "║      Guards: ";
                    if (trans.clockGuard.empty())
                        std::cout << "(none)" << std::endl;
                    else
                    {
                        std::cout << std::endl;
                        for (const auto &guard: trans.clockGuard)
                        {
                            std::cout << "║        " << guard.clock << " " << guard.guardOperator
                                    << " " << guard.comparingConstant << std::endl;
                        }
                    }

                    // Print resets
                    std::cout << "║      Resets: ";
                    if (trans.clocksToReset.empty())
                        std::cout << "(none)" << std::endl;
                    else
                    {
                        for (size_t j = 0; j < trans.clocksToReset.size(); ++j)
                        {
                            std::cout << trans.clocksToReset[j];
                            if (j < trans.clocksToReset.size() - 1)
                                std::cout << ", ";
                        }
                        std::cout << std::endl;
                    }
                }
            }
            std::cout << "╚══════════════════════════════════════════════════" << std::endl;
        }
    };
} // namespace timed_automaton::parser


#endif //PARSER_ON_SUCCESS_HANDLERS_H
