#ifndef RULE_CLASSES_H
#define RULE_CLASSES_H

#include <iostream>
#include <string>

namespace timed_automaton::parser {
    struct on_success_handler {
        template<typename T, typename Iterator, typename Context>
        static void on_success(Iterator const &first, Iterator const &last, T &, Context const &)
        {
            const std::string input(first, last);
            std::cout << "[PARSER] Matched:\n" << input << std::endl;
        }
    };

    struct loc_pair_class : on_success_handler {
    };

    struct loc_map_class : on_success_handler {
    };

    struct arena_loc_class : on_success_handler {
    };

    struct arena_loc_pair_class : on_success_handler {
    };

    struct arena_loc_map_class : on_success_handler {
    };

    struct guard_class : on_success_handler {
    };

    struct transition_class : on_success_handler {
    };

    struct timedAutomaton_class : on_success_handler {
    };

    struct timedArena_class : on_success_handler {
    };
} // namespace timed_automaton::parser

#endif //RULE_CLASSES_H
