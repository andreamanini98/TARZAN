#ifndef TARZAN_RTS_H
#define TARZAN_RTS_H

#include "Region.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/enums/state_space_exploration_enum.h"


namespace region
{
    class RTS
    {
    protected:
        // The size of this map corresponds to the number of 'automaton' clocks.
        std::unordered_map<std::string, int> clocksIndices{};

        std::unordered_map<std::string, int> locationsToInt{};

        std::vector<int> maxConstants{};

        std::vector<int> initialLocations{};

        std::vector<std::vector<transition>> outTransitions{};

        std::vector<std::vector<transition>> inTransitions{};

        std::vector<Region> initialRegions{};

        absl::flat_hash_map<int, std::vector<timed_automaton::ast::clockConstraint>> invariants{};

        absl::flat_hash_set<int> urgentLocations{};


    public:
        explicit RTS(const timed_automaton::ast::timedAutomaton &automaton)
        {
            clocksIndices = automaton.getClocksIndices();
            locationsToInt = automaton.mapLocationsToInt();
            maxConstants = automaton.getMaxConstants(clocksIndices);
            initialLocations = automaton.getInitialLocations(locationsToInt);
            outTransitions = automaton.getOutTransitions(locationsToInt);
            inTransitions = automaton.getInTransitions(locationsToInt);
            invariants = automaton.getInvariants(locationsToInt);
            urgentLocations = automaton.getUrgentLocations(locationsToInt);

            const int numOfClocks = static_cast<int>(clocksIndices.size());
            const auto &variables = automaton.getVariables();

            for (const int loc: initialLocations)
                initialRegions.emplace_back(numOfClocks, loc, variables);
        }


        RTS() = default;


        /**
         * @brief Computes whether a region with a location equal to targetLocation is reachable from the set of initial regions.
         *
         * @param intVarConstr a vector of clock constraints that act as integer variables constraints.
         *                     Instead of a clock, each clock constraint contains the name of an integer variable.
         * @param goalClockConstraints constraints that must hold for the goal region to be reachable.
         * @param targetLocation the location of the region that must be reached.
         * @param explorationTechnique determines the state space exploration technique (e.g., BFS, DFS).
         * @return a vector containing the target region if it is reachable, an empty vector otherwise.
         *
         * @warning The intVarConstr is a vector of clock constraints, although here, instead of clocks, integer variables must be used.
         */
        [[nodiscard]] std::vector<Region> forwardReachability(const std::vector<timed_automaton::ast::clockConstraint> &intVarConstr,
                                                              const std::vector<timed_automaton::ast::clockConstraint> &goalClockConstraints,
                                                              int targetLocation,
                                                              ssee explorationTechnique) const;


        /**
         * @brief Computes whether a region with a location equal to targetLocation is reachable from the set of initial regions.
         *
         * @param intVarOrClockConstr a vector of clock constraints that act either as integer variables or clock constraints.
         * @param targetLocation the location of the region that must be reached.
         * @param explorationTechnique determines the state space exploration technique (e.g., BFS, DFS).
         * @param isIntVarConstr determines whether intVarOrClockConstr is treated as a variable (true) or clock (false) vector of constraints.
         * @return a vector containing the target region if it is reachable, an empty vector otherwise.
         *
         * @warning The intVarConstr is a vector of clock constraints, although here, instead of clocks, integer variables must be used.
         */
        [[nodiscard]] std::vector<Region> forwardReachability(const std::vector<timed_automaton::ast::clockConstraint> &intVarOrClockConstr,
                                                              int targetLocation,
                                                              ssee explorationTechnique,
                                                              bool isIntVarConstr) const;


        /**
         * @brief Computes whether a region with a location equal to targetLocation is reachable from the set of initial regions.
         *
         * @param targetLocation the location of the region that must be reached.
         * @param explorationTechnique determines the state space exploration technique (e.g., BFS, DFS).
         * @return a vector containing the target region if it is reachable, an empty vector otherwise.
         */
        [[nodiscard]] std::vector<Region> forwardReachability(int targetLocation,
                                                              ssee explorationTechnique) const;


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
        [[nodiscard]] const std::unordered_map<std::string, int> &getClocksIndices() const { return clocksIndices; }
        [[nodiscard]] const std::unordered_map<std::string, int> &getLocationsToInt() const { return locationsToInt; }
        [[nodiscard]] const std::vector<int> &getMaxConstants() const { return maxConstants; }
        [[nodiscard]] const std::vector<int> &getInitialLocations() const { return initialLocations; }
        [[nodiscard]] const std::vector<std::vector<transition>> &getOutTransitions() const { return outTransitions; }
        [[nodiscard]] const std::vector<std::vector<transition>> &getInTransitions() const { return inTransitions; }
        [[nodiscard]] const absl::flat_hash_map<int, std::vector<timed_automaton::ast::clockConstraint>> &getInvariants() const { return invariants; }
        [[nodiscard]] const absl::flat_hash_set<int> &getUrgentLocations() const { return urgentLocations; }


        [[nodiscard]] std::string to_string() const;
    };
}


#endif //TARZAN_RTS_H
