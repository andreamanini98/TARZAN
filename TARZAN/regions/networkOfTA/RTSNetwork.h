#ifndef TARZAN_RTSNETWORK_H
#define TARZAN_RTSNETWORK_H

#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/networkOfTA/NetworkRegion.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/utilities/partition_utilities.h"


// TODO: devi aggiustare l'implementazione dei successori qui e nella classe RTS.

// TODO: devi fare in modo che ogni clock abbia la sua costante massima.

// TODO: devi aggiungere la possibilità di avere degli interi nelle regioni normali (prima pensare bene a come farlo però).


// Regarding the invariants in Timed Automata:
// it is enough to take the invariant and combine it with the guard (using conjunction) on all transitions entering and leaving the location
// that has the invariant. This approach works if you only need to compute reachability (as in our case), but it may not be enough for other purposes,
// such as handling Buchi acceptance conditions.

namespace networkOfTA
{
    class RTSNetwork
    {
        std::vector<timed_automaton::ast::timedAutomaton> automata;

        std::vector<int> maxConstants{};

        // The size of this map's element corresponds to the number of the i-th 'automaton' clocks.
        std::vector<std::unordered_map<std::string, int>> clocksIndices{};

        std::vector<std::unordered_map<std::string, int>> locationsToInt{};

        std::vector<std::vector<int>> initialLocations{};

        std::vector<std::vector<std::vector<transition>>> outTransitions{};

        std::vector<std::vector<std::vector<transition>>> inTransitions{};

        std::vector<NetworkRegion> initialRegions{};


    public:
        explicit RTSNetwork(const std::vector<timed_automaton::ast::timedAutomaton> &automata) : automata(automata)
        {
            for (int i = 0; i < static_cast<int>(automata.size()); i++)
            {
                const timed_automaton::ast::timedAutomaton &automaton = automata[i];

                maxConstants.emplace_back(automaton.getMaxConstant());
                clocksIndices.emplace_back(automaton.getClocksIndices());
                locationsToInt.emplace_back(automaton.mapLocationsToInt());
                initialLocations.emplace_back(automaton.getInitialLocations(locationsToInt[i]));
                outTransitions.emplace_back(automaton.getOutTransitions(locationsToInt[i]));
                inTransitions.emplace_back(automaton.getInTransitions(locationsToInt[i]));
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


        // TODO: questo va modificato, vedi nel file .cpp qualche indizio.
        [[nodiscard]] std::vector<NetworkRegion> buildRegionGraphForeword(const std::vector<int> &targetLocations) const;


        [[nodiscard]] std::string toString() const;


        // Getters.
        [[nodiscard]] std::vector<NetworkRegion> getInitialRegions() const { return initialRegions; }
        [[nodiscard]] std::vector<int> getMaxConstants() const { return maxConstants; }
    };
}

#endif //TARZAN_RTSNETWORK_H
