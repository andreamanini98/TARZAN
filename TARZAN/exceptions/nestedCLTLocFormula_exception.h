#ifndef TARZAN_NESTEDCLTLOCFORMULA_EXCEPTION_H
#define TARZAN_NESTEDCLTLOCFORMULA_EXCEPTION_H

#include <stdexcept>


namespace region
{
    class NestedCLTLocFormulaException final : public std::runtime_error
    {
    public:
        explicit NestedCLTLocFormulaException(const std::string &message) : std::runtime_error(message) {}
    };
}

#endif //TARZAN_NESTEDCLTLOCFORMULA_EXCEPTION_H
