#ifndef TARZAN_BOOLEAN_OP_ENUM_H
#define TARZAN_BOOLEAN_OP_ENUM_H

#include <string>


// Enumeration containing boolean operators.
enum boolean_op { AND, OR };


// Convert boolean_op to string.
inline std::string boolean_op_to_string(const boolean_op op)
{
    switch (op)
    {
        case AND: return "&&";
        case OR: return "||";
        default: return "invalid_boolean_op";
    }
}


// Stream operator for convenient printing.
inline std::ostream &operator<<(std::ostream &os, const boolean_op op)
{
    return os << boolean_op_to_string(op);
}

#endif //TARZAN_BOOLEAN_OP_ENUM_H
