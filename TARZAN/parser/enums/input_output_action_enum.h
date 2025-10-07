#ifndef TARZAN_INPUT_OUTPUT_ACTION_ENUM_H
#define TARZAN_INPUT_OUTPUT_ACTION_ENUM_H

#include <string>


// Enumeration containing symbols used to specify input and output actions.
enum in_out_act { OUTACT, INACT };


// Operator to convert in_out_act to string
inline std::string in_out_act_to_string(const in_out_act ioa)
{
    switch (ioa)
    {
        case OUTACT: return "!";
        case INACT: return "?";
        default: return "Invalid in_out_act symbol";
    }
}


// Stream operator for convenient printing
inline std::ostream &operator<<(std::ostream &os, const in_out_act ioa)
{
    return os << in_out_act_to_string(ioa);
}

#endif //TARZAN_INPUT_OUTPUT_ACTION_ENUM_H
