#ifndef TARZAN_CLTLOC_OP_ENUM_H
#define TARZAN_CLTLOC_OP_ENUM_H

#include <string>


// Enumeration containing binary operators used in CLTLoc formulae.
enum binary_cltloc_op { UNTIL };


// Convert binary_cltloc_op to string.
inline std::string binary_cltloc_op_to_string(const binary_cltloc_op op)
{
    switch (op)
    {
        case UNTIL: return "U";
        default: return "invalid_binary_cltloc_op";
    }
}


// Stream operator for convenient printing.
inline std::ostream &operator<<(std::ostream &os, const binary_cltloc_op op)
{
    return os << binary_cltloc_op_to_string(op);
}


// ---


// Enumeration containing unary operators used in CLTLoc formulae.
enum unary_cltloc_op { BOX };


// Convert unary_cltloc_op to string.
inline std::string unary_cltloc_op_to_string(const unary_cltloc_op op)
{
    switch (op)
    {
        case BOX: return "[]";
        default: return "invalid_unary_cltloc_op";
    }
}


// Stream operator for convenient printing.
inline std::ostream &operator<<(std::ostream &os, const unary_cltloc_op op)
{
    return os << unary_cltloc_op_to_string(op);
}

#endif //TARZAN_CLTLOC_OP_ENUM_H
