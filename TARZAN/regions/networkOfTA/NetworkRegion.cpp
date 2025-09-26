#include "NetworkRegion.h"

#define NETWORKREGION_DEBUG


networkOfTA::NetworkRegion networkOfTA::NetworkRegion::getImmediateDelaySuccessor(const std::vector<int> &maxConstants) const
{
    NetworkRegion reg = clone();

    if (!isAorC.empty())
    {
        // Map that (if not empty) will be inserted in the isB deque front, meaning that clocks exiting from the units of these regions have the smallest fractional part.
        absl::flat_hash_map<int, boost::dynamic_bitset<>> newClockOrdering{};

        for (const int regIndex: isAorC)
        {
            region::Region &regionToDelay = reg.regions[regIndex];

            // Used later to check whether some clocks must be inserted into the newClockOrdering map.
            const boost::dynamic_bitset<> originalX0 = regionToDelay.getX0();

            // Immediate delay successor of the region corresponding to index regIndex.
            const region::Region &successor = regionToDelay.getImmediateDelaySuccessor(maxConstants[regIndex]);

            // Updating regions with the newly computed successor.
            regionToDelay = successor;

            // If the resulting region has at least one bounded clock set, we must check which clocks are still bounded.
            // ReSharper disable once CppTooWideScopeInitStatement
            std::deque<boost::dynamic_bitset<>> newBounded = regionToDelay.getBounded();

            if (!newBounded.empty())
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const boost::dynamic_bitset<> boundedClocksThatLeftTheUnit = originalX0 & newBounded.front();

                // If some clocks are still bounded after leaving the unit, they will have the smallest fractional part.
                // The fact that clock sets are partitions ensures that, if all clocks became unbounded, this bitset has only bits set to 0 inside.
                if (boundedClocksThatLeftTheUnit.any())
                    newClockOrdering[regIndex] = boundedClocksThatLeftTheUnit;
            }
        }

        // All regions have been processed, and they do not belong to either class A or C anymore (no region is either of class A or C anymore).
        reg.isAorC.clear();

        // Updating the isB deque.
        if (!newClockOrdering.empty())
            reg.isB.emplace_front(newClockOrdering);
    } else
    {
        if (!isB.empty())
        {
            // Getting information about which regions must be delayed.
            // ReSharper disable once CppTooWideScopeInitStatement
            const absl::flat_hash_map<int, boost::dynamic_bitset<>> &clockOrdering = reg.isB.back();

            for (const auto &regIndex: clockOrdering | std::views::keys)
            {
                region::Region &regionToDelay = reg.regions[regIndex];

                // Immediate delay successor of the region corresponding to index regIndex.
                const region::Region &successor = regionToDelay.getImmediateDelaySuccessor(maxConstants[regIndex]);

                // Updating regions with the newly computed successor.
                regionToDelay = successor;

                // The region now is either of class A or C.
                reg.isAorC.push_back(regIndex);
            }

            // Removing clockOrdering from isB, since these clocks are now all inside x0 in their respective region.
            reg.isB.pop_back();
        }
    }

    return reg;
}
