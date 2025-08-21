#include "Region.h"

#include<sstream>


std::string Region::toString() const
{
    std::ostringstream oss;
    oss << "Region {\n";
    oss << "  q: " << q << "\n";
    oss << "  h: [";
    for (const auto &i: h)
        oss << "  " << i;
    oss << "  ]\n";
    oss << "  bounded: [\n";
    for (const auto &i: bounded)
        oss << "    " << i << "\n";
    oss << "  ]\n";
    oss << "  unbounded: [\n";
    for (const auto &i: unbounded)
        oss << "    " << i << "\n";
    oss << "  ]\n";
    oss << "  x0: " << x0 << "\n";
    oss << "}";
    return oss.str();
}
