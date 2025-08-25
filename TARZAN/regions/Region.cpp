#include "Region.h"

#include <iostream>
#include <sstream>


Region Region::getImmediateDelaySuccessor(const int maxConstant) const
{
    Region res = clone();
    const int numOfClocks = getNumberOfClocks();

    if (bounded.empty() && x0.none())
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
                res.h[i]++;

        res.x0 |= lastBoundedSet;
        res.bounded.pop_back();
    }

    return res;
}


std::vector<Region> Region::getImmediateDelayPredecessors() const
{
    std::vector<Region> res;
    const int numOfClocks = getNumberOfClocks();

    Region r0 = clone();

    if (bounded.empty() && x0.none())
    {
        r0.x0 |= r0.unbounded.front();
        r0.unbounded.pop_front();
    } else if (x0.any())
    {
        for (int i = 0; i < numOfClocks; i++)
        {
            if (r0.x0.test(numOfClocks - 1 - i))
            {
                r0.h[i]--;
                // If the region cannot have a delay predecessor, return an empty std::vector.
                if (r0.h[i] < 0)
                    return res;
            }
        }
        r0.bounded.push_back(r0.x0);
        r0.x0 &= boost::dynamic_bitset<>(numOfClocks);
    } else
    {
        r0.x0 |= r0.bounded.front();
        r0.bounded.pop_front();

        if (!unbounded.empty())
        {
            Region r1 = clone();
            r1.x0 |= r1.unbounded.front();
            r1.unbounded.pop_front();
            res.push_back(r1);

            Region r2 = r1.clone();
            r2.x0 |= r2.bounded.front();
            r2.bounded.pop_front();
            res.push_back(r2);
        }
    }

    res.push_back(r0);
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
