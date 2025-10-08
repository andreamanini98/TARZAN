#ifndef TARZAN_ARITHMETIC_OPERATOR_ENUM_H
#define TARZAN_ARITHMETIC_OPERATOR_ENUM_H

#include <string>


// Enumeration containing arithmetic operators used in expressions.
enum arithmetic_op { ADD, SUB, MUL, DIV };


// Convert arithmetic_op to string.
inline std::string arithmetic_op_to_string(const arithmetic_op op)
{
    switch (op)
    {
        case ADD: return "+";
        case SUB: return "-";
        case MUL: return "*";
        case DIV: return "/";
        default: return "invalid_arithmetic_op";
    }
}


// Stream operator for convenient printing.
inline std::ostream &operator<<(std::ostream &os, const arithmetic_op op)
{
    return os << arithmetic_op_to_string(op);
}


// ---


// Enumeration containing assignment operators used in expressions.
enum assignment_op { ASS };

// Convert assignment_op to string.
inline std::string assignment_op_to_string(const assignment_op op)
{
    switch (op)
    {
        case ASS: return "=";
        default: return "invalid_assignment_op";
    }
}


// Stream operator for convenient printing.
inline std::ostream &operator<<(std::ostream &os, const assignment_op op)
{
    return os << assignment_op_to_string(op);
}

#endif //TARZAN_ARITHMETIC_OPERATOR_ENUM_H
