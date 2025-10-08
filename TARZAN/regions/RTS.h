#ifndef TARZAN_RTS_H
#define TARZAN_RTS_H

#include "Region.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/enums/state_space_exploration_enum.h"


namespace region
{
    class RTS
    {
        timed_automaton::ast::timedAutomaton automaton;

        // The size of this map corresponds to the number of 'automaton' clocks.
        std::unordered_map<std::string, int> clocksIndices{};

        std::unordered_map<std::string, int> locationsToInt{};

        std::vector<int> maxConstants{};

        std::vector<int> initialLocations{};

        std::vector<std::vector<transition>> outTransitions{};

        std::vector<std::vector<transition>> inTransitions{};

        std::vector<Region> initialRegions{};

        absl::flat_hash_map<int, std::vector<timed_automaton::ast::clockConstraint>> invariants{};


    public:
        explicit RTS(const timed_automaton::ast::timedAutomaton &automaton) : automaton(automaton)
        {
            clocksIndices = automaton.getClocksIndices();
            locationsToInt = automaton.mapLocationsToInt();
            maxConstants = automaton.getMaxConstants(clocksIndices);
            initialLocations = automaton.getInitialLocations(locationsToInt);
            outTransitions = automaton.getOutTransitions(locationsToInt);
            inTransitions = automaton.getInTransitions(locationsToInt);
            invariants = automaton.getInvariants(locationsToInt);

            const int numOfClocks = static_cast<int>(clocksIndices.size());
            for (const int loc: initialLocations)
                initialRegions.emplace_back(numOfClocks, loc);
        }


        /**
         * @brief Computes whether a region with a location equal to targetLocation is reachable from the set of initial regions.
         *
         * @param targetLocation the location of the region that must be reached.
         * @param explorationTechnique determines the state space exploration technique (e.g., BFS, DFS).
         * @return a vector containing the target region if it is reachable, an empty vector otherwise.
         */
        [[nodiscard]] std::vector<Region> forwardReachability(int targetLocation, ssee explorationTechnique) const;


        /**
         * @brief Computes whether an initial region is reachable from a set of starting regions.
         *
         * @param startingRegions the regions from which to start the backward reachability analysis.
         * @param explorationTechnique determines the state space exploration technique (e.g., BFS, DFS).
         * @return a vector containing an initial region if it is reachable, an empty vector otherwise.
         */
        [[nodiscard]] std::vector<Region> backwardReachability(const std::vector<Region> &startingRegions, ssee explorationTechnique) const;


        // Getters.
        [[nodiscard]] std::vector<Region> getInitialRegions() const { return initialRegions; }
    };
}


#endif //TARZAN_RTS_H
