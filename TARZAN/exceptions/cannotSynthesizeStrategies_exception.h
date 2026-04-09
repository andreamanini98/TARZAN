#ifndef TARZAN_CANNOTSYNTHESIZESTRATEGIES_EXCEPTION_H
#define TARZAN_CANNOTSYNTHESIZESTRATEGIES_EXCEPTION_H

#include <stdexcept>


namespace region
{
    class CannotSynthesizeStrategiesException final : public std::runtime_error
    {
    public:
        explicit CannotSynthesizeStrategiesException(const std::string &message) : std::runtime_error(message) {}
    };
}

#endif //TARZAN_CANNOTSYNTHESIZESTRATEGIES_EXCEPTION_H
