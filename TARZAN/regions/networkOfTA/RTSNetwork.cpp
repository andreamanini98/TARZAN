#include "RTSNetwork.h"

#include "absl/container/flat_hash_set.h"


std::vector<networkOfTA::NetworkRegion> networkOfTA::RTSNetwork::buildRegionGraphForeword(const std::vector<int> &targetLocations) const
{
    std::vector<NetworkRegion> toProcess{};
    std::vector<NetworkRegion> result{};

    absl::flat_hash_set<NetworkRegion, NetworkRegionHash> regionsHashMap{};

    for (const auto &init: getInitialRegions())
    {
        result.push_back(init);
        toProcess.push_back(init);
        regionsHashMap.insert(init);
    }

    unsigned long long int totalregions = 0;

    const auto start = std::chrono::high_resolution_clock::now();

    while (!toProcess.empty())
    {
        std::cout << "\nTOPROCESSSIZE: " << toProcess.size() << std::endl;
        std::cout << "\nTOTALREGIONS: " << totalregions << std::endl;

        std::vector<NetworkRegion> successors{};

        const NetworkRegion &currentRegion = toProcess.back();

        std::cout << "\nCurrent network region:\n" << currentRegion.toString() << std::endl;

        bool stop = true;

        for (int i = 0; i < static_cast<int>(targetLocations.size()); i++)
            stop = stop && currentRegion.getRegions()[i].getLocation() == targetLocations[i];

        if (stop)
        {
            const auto end = std::chrono::high_resolution_clock::now();
            std::cout << "Total number of computed regions: " << totalregions << std::endl;
            std::cout << currentRegion.toString() << std::endl;
            std::cout << "GOAL REGION IS REACHABLE!\n";

            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;
            std::vector<NetworkRegion> regionToReturn{};
            regionToReturn.push_back(currentRegion);
            return regionToReturn;
        }

        const NetworkRegion delaySuccessor = currentRegion.getImmediateDelaySuccessor(maxConstants);

        std::vector<std::vector<transition>> transitions{};
        for (int i = 0; i < static_cast<int>(currentRegion.getRegions().size()); i++)
        {
            transitions.push_back(outTransitions[i][currentRegion.getRegions()[i].getLocation()]);
        }

        const std::vector<NetworkRegion> discreteSuccessors = currentRegion.getImmediateDiscreteSuccessors(transitions, clocksIndices, locationsToInt);

        totalregions += discreteSuccessors.size() + 1;

        // Removing now since we do not need it anymore.
        toProcess.pop_back();

        if (!regionsHashMap.contains(delaySuccessor))
        {
            regionsHashMap.insert(delaySuccessor);
            toProcess.push_back(delaySuccessor);
        }
        for (const auto &reg: discreteSuccessors)
        {
            if (!regionsHashMap.contains(reg))
            {
                regionsHashMap.insert(reg);
                toProcess.push_back(reg);
            }
        }
    }

    return result;
}


std::string networkOfTA::RTSNetwork::toString() const
{
    std::ostringstream oss;

    oss << "RTSNetwork {\n";

    // Display automata information.
    oss << "  Automata (" << automata.size() << "):\n";
    for (size_t i = 0; i < automata.size(); i++)
        oss << "    [" << i << "]: " << automata[i].name << "\n";

    // Display max constants.
    oss << "  Max Constants (" << maxConstants.size() << "): [";
    for (size_t i = 0; i < maxConstants.size(); i++)
    {
        oss << maxConstants[i];
        if (i < maxConstants.size() - 1)
            oss << ", ";
    }
    oss << "]\n";

    // Display clocks info.
    oss << "  Clocks:\n";
    for (size_t i = 0; i < clocksIndices.size(); i++)
    {
        oss << "    Automaton [" << i << "] (" << clocksIndices[i].size() << " clocks): {";
        bool first = true;
        for (const auto &[clock, index]: clocksIndices[i])
        {
            if (!first)
                oss << ", ";
            first = false;
            oss << clock << " -> " << index;
        }
        oss << "}\n";
    }

    // Display locations.
    oss << "  Locations:\n";
    for (size_t i = 0; i < locationsToInt.size(); i++)
    {
        oss << "    Automaton [" << i << "] (" << locationsToInt[i].size() << " locations): {";
        bool first = true;
        for (const auto &[loc, index]: locationsToInt[i])
        {
            if (!first)
                oss << ", ";
            first = false;
            oss << loc << " -> " << index;
        }
        oss << "}\n";
    }

    // Display initial locations.
    oss << "  Initial Locations:\n";
    for (size_t i = 0; i < initialLocations.size(); i++)
    {
        oss << "    Automaton [" << i << "]: [";
        for (size_t j = 0; j < initialLocations[i].size(); j++)
        {
            oss << initialLocations[i][j];
            if (j < initialLocations[i].size() - 1)
                oss << ", ";
        }
        oss << "]\n";
    }

    // Display initial network regions.
    oss << "  Initial Regions (" << initialRegions.size() << "):\n";
    for (size_t i = 0; i < initialRegions.size(); i++)
    {
        oss << "    [" << i << "]:\n";
        // Indent the NetworkRegion's toString output.
        std::istringstream regionStream(initialRegions[i].toString());
        std::string line;
        while (std::getline(regionStream, line))
            if (!line.empty())
                oss << "      " << line << "\n";
    }

    // Display transitions counts
    oss << "  Transitions:\n";
    for (size_t i = 0; i < outTransitions.size(); i++)
    {
        int totalOut = 0;
        for (const auto &locTransitions: outTransitions[i])
            totalOut += static_cast<int>(locTransitions.size());

        int totalIn = 0;
        for (const auto &locTransitions: inTransitions[i])
            totalIn += static_cast<int>(locTransitions.size());

        oss << "    Automaton [" << i << "]: "
                << outTransitions[i].size() << " locations, "
                << totalOut << " outgoing transitions, "
                << totalIn << " incoming transitions\n";
    }

    oss << "}\n";
    return oss.str();
}
