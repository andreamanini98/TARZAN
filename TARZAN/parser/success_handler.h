#ifndef SUCCESS_HANDLER_H
#define SUCCESS_HANDLER_H

#include <iostream>
#include <regex>

struct success_handler
{
    template<typename T, typename Iterator, typename Context>
    static void on_success(Iterator const &first, Iterator const &last, T &, Context const &)
    {
        // Default implementation returns demangled type name
        const char *name = typeid(T).name();
        std::string type_name(name);

        const std::string input(first, last);

        std::cout << "[PARSER] Successful match!" << std::endl;
        std::cout << "Rule: " << name << std::endl;
        std::cout << "Parsed input:\n" << input << std::endl;
    }
};

#endif //SUCCESS_HANDLER_H
