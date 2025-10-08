#ifndef TARZAN_RTSNETWORK_H
#define TARZAN_RTSNETWORK_H

#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/networkOfTA/NetworkRegion.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/regions/enums/state_space_exploration_enum.h"
#include "TARZAN/utilities/partition_utilities.h"

// TODO: devi aggiungere la possibilità di avere degli interi nelle regioni normali (prima pensare bene a come farlo però).


namespace networkOfTA
{
    class RTSNetwork
    {
        std::vector<timed_automaton::ast::timedAutomaton> automata;

        // The size of this map's element corresponds to the number of the i-th 'automaton' clocks.
        std::vector<std::unordered_map<std::string, int>> clocksIndices{};

        std::vector<std::unordered_map<std::string, int>> locationsToInt{};

        std::vector<std::vector<int>> maxConstants{};

        std::vector<std::vector<int>> initialLocations{};

        std::vector<std::vector<std::vector<transition>>> outTransitions{};

        std::vector<std::vector<std::vector<transition>>> inTransitions{};

        std::vector<NetworkRegion> initialRegions{};

        std::vector<absl::flat_hash_map<int, std::vector<timed_automaton::ast::clockConstraint>>> invariants{};


    public:
        explicit RTSNetwork(const std::vector<timed_automaton::ast::timedAutomaton> &automata) : automata(automata)
        {
            for (int i = 0; i < static_cast<int>(automata.size()); i++)
            {
                const timed_automaton::ast::timedAutomaton &automaton = automata[i];

                clocksIndices.emplace_back(automaton.getClocksIndices());
                locationsToInt.emplace_back(automaton.mapLocationsToInt());
                maxConstants.emplace_back(automaton.getMaxConstants(clocksIndices[i]));
                initialLocations.emplace_back(automaton.getInitialLocations(locationsToInt[i]));
                outTransitions.emplace_back(automaton.getOutTransitions(locationsToInt[i]));
                inTransitions.emplace_back(automaton.getInTransitions(locationsToInt[i]));
                invariants.emplace_back(automaton.getInvariants(locationsToInt[i]));
            }

            // ReSharper disable once CppTooWideScopeInitStatement
            const std::vector<std::vector<int>> &initialLocationsCartesianProduct = vectorsCartesianProduct(initialLocations);

            for (const auto &cartesianProduct: initialLocationsCartesianProduct)
            {
                std::vector<region::Region> initRegs{};

                for (int i = 0; i < static_cast<int>(cartesianProduct.size()); i++)
                    initRegs.emplace_back(static_cast<int>(clocksIndices[i].size()), cartesianProduct[i]);

                initialRegions.emplace_back(initRegs, true);
            }
        }


        /**
         * @brief Computes whether a network region with locations equal to targetLocs is reachable from the set of initial network regions.
         *
         * @param targetLocs the locations of the network region that must be reached.
         * @param explorationTechnique determines the state space exploration technique (e.g., BFS, DFS).
         * @return a vector containing the target network region if it is reachable, an empty vector otherwise.
         */
        [[nodiscard]] std::vector<NetworkRegion> forwardReachability(const std::vector<int> &targetLocs, ssee explorationTechnique) const;


        [[nodiscard]] std::string toString() const;


        // Getters.
        [[nodiscard]] const std::vector<NetworkRegion> &getInitialRegions() const { return initialRegions; }
        [[nodiscard]] const std::vector<std::vector<int>> &getMaxConstants() const { return maxConstants; }
    };
}

#endif //TARZAN_RTSNETWORK_H
