#ifndef TARZAN_RTS_H
#define TARZAN_RTS_H

#include <utility>

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


    public:
        explicit RTS(const timed_automaton::ast::timedAutomaton &automaton) : automaton(automaton)
        {
            maxConstant = automaton.getMaxConstant();
            clocksIndices = automaton.getClocksIndices();
            locationsToInt = automaton.mapLocationsToInt();
            initialLocations = automaton.getInitialLocations(locationsToInt);
            outTransitions = automaton.getOutTransitions(locationsToInt);
        }


        /**
         * @return the initial regions of the RTS 'automaton' attribute.
         */
        [[nodiscard]] std::vector<Region> getInitialRegions() const;
    };
}


#endif //TARZAN_RTS_H
