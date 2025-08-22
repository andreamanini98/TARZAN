#include "Region.h"

#include <iostream>
#include<sstream>


Region Region::getImmediateDelaySuccessor(const int maxConstant) const
{
    Region res = clone();
    const int numOfClocks = getNumberOfClocks();

    if (x0.none() && bounded.empty())
        return res;
    if (x0.any())
    {
        boost::dynamic_bitset<> xTmp(numOfClocks);
        boost::dynamic_bitset<> xOob(numOfClocks);

        for (int i = 0; i < numOfClocks; i++)
        {
            if (res.x0.test(numOfClocks - 1 - i))
            {
                if (res.h[i] == maxConstant)
                    xOob.set(numOfClocks - 1 - i, true);
                else
                    xTmp.set(numOfClocks - 1 - i, true);
            }
        }
        res.x0 &= boost::dynamic_bitset<>(numOfClocks);

        if (xTmp.any())
            res.bounded.push_front(xTmp);
        if (xOob.any())
            res.unbounded.push_front(xOob);
    } else
    {
        const boost::dynamic_bitset<> &lastBoundedSet = res.bounded.back();

        for (int i = 0; i < numOfClocks; i++)
            if (lastBoundedSet.test(numOfClocks - 1 - i))
                res.h[i] += 1;

        res.x0 |= lastBoundedSet;
        res.bounded.pop_back();
    }

    return res;
}


std::string Region::toString() const
{
    std::ostringstream oss;
    oss << "Region\n";
    oss << "  q: " << q << "\n";
    oss << "  h: [";
    if (h != nullptr)
    {
        const int *it = h;
        for (int i = 0; i < static_cast<int>(x0.size()); i++)
        {
            oss << "  " << *it;
            it++;
        }
    }
    oss << "  ]\n";
    oss << "  unbounded: [";
    for (const auto &i: unbounded)
        oss << "  " << i;
    oss << "  ]\n";
    oss << "  x0:        [  " << x0 << "  ]\n";
    oss << "  bounded:   [";
    for (const auto &i: bounded)
        oss << "  " << i;
    oss << "  ]\n";
    return oss.str();
}
