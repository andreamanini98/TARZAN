#ifndef TARZAN_EMPTYCONJUNCTIONOFFORMULAE_EXCEPTION_H
#define TARZAN_EMPTYCONJUNCTIONOFFORMULAE_EXCEPTION_H

#include <stdexcept>


namespace region
{
    class EmptyConjunctionOfFormulaeException final : public std::runtime_error
    {
    public:
        explicit EmptyConjunctionOfFormulaeException(const std::string &message) : std::runtime_error(message) {}
    };
}

#endif //TARZAN_EMPTYCONJUNCTIONOFFORMULAE_EXCEPTION_H
