#ifndef COMPARISON_OP_ENUM_H
#define COMPARISON_OP_ENUM_H

#include <iostream>

// Enumeration containing comparison operators used in guards.
enum comparison_op { LT, LE, EQ, GE, GT };


// Operator to convert comp_op to string
inline std::string comparison_op_to_string(const comparison_op op)
{
    switch (op)
    {
        case LT: return "<";
        case LE: return "<=";
        case EQ: return "=";
        case GE: return ">=";
        case GT: return ">";
        default: return "?";
    }
}


// Stream operator for convenient printing
inline std::ostream &operator<<(std::ostream &os, const comparison_op op)
{
    return os << comparison_op_to_string(op);
}

#endif //COMPARISON_OP_ENUM_H
