#ifndef TARZAN_PLAYERS_ENUM_H
#define TARZAN_PLAYERS_ENUM_H

#include <string>


// Enumeration containing symbols used to specify players in Timed Arenas.
enum players_sym { CONTROLLER, ENVIRONMENT };


// Convert players_sym to string.
inline std::string players_sym_to_string(const players_sym ps)
{
    switch (ps)
    {
        case CONTROLLER: return "c";
        case ENVIRONMENT: return "e";
        default: return "invalid_players_sym";
    }
}


// Stream operator for convenient printing.
inline std::ostream &operator<<(std::ostream &os, const players_sym ps)
{
    return os << players_sym_to_string(ps);
}

#endif //TARZAN_PLAYERS_ENUM_H