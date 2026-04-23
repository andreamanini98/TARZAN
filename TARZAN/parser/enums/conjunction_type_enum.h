#ifndef TARZAN_CONJUNCTION_TYPE_ENUM_H
#define TARZAN_CONJUNCTION_TYPE_ENUM_H

#include <string>


// Enumeration containing the type of conjunctionOfGeneralCLTLocFormulae formulae.
enum conjunction_type { AND_GENERAL, AND_NEXT, NESTED_UNTIL};

// Convert conjunction_type to string.
inline std::string conjunction_type_to_string(const conjunction_type ct)
{
    switch (ct)
    {
        case AND_GENERAL: return "AND_GENERAL";
        case AND_NEXT: return "AND_NEXT";
        case NESTED_UNTIL: return "NESTED_UNTIL";
        default: return "conjunction_type";
    }
}


// Stream operator for convenient printing.
inline std::ostream &operator<<(std::ostream &os, const conjunction_type ct)
{
    return os << conjunction_type_to_string(ct);
}

#endif //TARZAN_CONJUNCTION_TYPE_ENUM_H
