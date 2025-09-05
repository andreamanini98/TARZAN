#ifndef TARZAN_RTS_H
#define TARZAN_RTS_H

#include "Region.h"
#include "TARZAN/parser/ast.h"


namespace region
{
    class RTS
    {
        timed_automaton::ast::timedAutomaton automaton;

        int maxConstant{};

        // The size of this map corresponds to the number of 'automaton' clocks.
        std::unordered_map<std::string, int> clocksIndices{};

        std::unordered_map<std::string, int> locationsToInt{};

        std::vector<int> initialLocations{};

        std::vector<std::vector<transition>> outTransitions{};

        std::vector<Region> initialRegions{};


    public:
        explicit RTS(const timed_automaton::ast::timedAutomaton &automaton) : automaton(automaton)
        {
            maxConstant = automaton.getMaxConstant();
            clocksIndices = automaton.getClocksIndices();
            locationsToInt = automaton.mapLocationsToInt();
            initialLocations = automaton.getInitialLocations(locationsToInt);
            outTransitions = automaton.getOutTransitions(locationsToInt);

            const int numOfClocks = static_cast<int>(clocksIndices.size());
            for (const int loc: initialLocations)
                initialRegions.emplace_back(numOfClocks, loc);
        }


        // TODO: questo va modificato, vedi nel file .cpp qualche indizio.
        [[nodiscard]] std::vector<Region> buildRegionGraphForeword() const;


        // Getters.
        [[nodiscard]] std::vector<Region> getInitialRegions() const { return initialRegions; }
    };
}


#endif //TARZAN_RTS_H
