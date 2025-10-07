#ifndef TARZAN_STATE_SPACE_EXPLORATION_ENUM_H
#define TARZAN_STATE_SPACE_EXPLORATION_ENUM_H

#include <string>


// Enumeration containing ways to explore the state space (ssee = state space exploration enum).
enum ssee { BFS, DFS };


// Operator to convert ssee to string.
inline std::string ssee_to_string(const ssee se)
{
    switch (se)
    {
        case BFS: return "BFS";
        case DFS: return "DFS";
        default: return "invalid";
    }
}


// Stream operator for convenient printing.
inline std::ostream &operator<<(std::ostream &os, const ssee se)
{
    return os << ssee_to_string(se);
}

#endif //TARZAN_STATE_SPACE_EXPLORATION_ENUM_H
