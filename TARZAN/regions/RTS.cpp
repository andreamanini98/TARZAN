#include "RTS.h"


std::vector<region::Region> region::RTS::buildRegionGraphForeword() const
{
    std::vector toProcess{ getInitialRegions() };
    std::vector result{ getInitialRegions() };

    while (!toProcess.empty())
    {
        std::vector<Region> successors{};

        Region currentRegion = toProcess.back();
        toProcess.pop_back();

        Region delaySuccessor = currentRegion.getImmediateDelaySuccessor(maxConstant);

        std::vector<transition> transitions = outTransitions[currentRegion.getLocation()];
        std::vector<Region> discreteSuccessors = currentRegion.getImmediateDiscreteSuccessors(transitions, clocksIndices, locationsToInt);

        if (std::ranges::find(result, delaySuccessor) == result.end())
        {
            result.push_back(delaySuccessor);
            toProcess.push_back(delaySuccessor);
        }
        for (const auto &reg: discreteSuccessors)
        {
            if (std::ranges::find(result, reg) == result.end())
            {
                result.push_back(reg);
                toProcess.push_back(reg);
            }
        }
    }

    return result;
}
