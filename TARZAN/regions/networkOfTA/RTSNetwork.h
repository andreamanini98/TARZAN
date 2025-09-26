#ifndef TARZAN_RTSNETWORK_H
#define TARZAN_RTSNETWORK_H

#include "absl/container/flat_hash_map.h"

#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/networkOfTA/NetworkRegion.h"
#include "TARZAN/parser/ast.h"
#include "TARZAN/utilities/partition_utilities.h"

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

        // Used to retrieve the region to which a clock belongs (the key corresponds to the clock's name, the vector to the regions).
        // The vector of integers is used in the case where multiple Timed Automata have clocks with the same name.
        absl::flat_hash_map<std::string, std::vector<int>> clocksToRegions{};

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

                for (const auto &clock: automaton.clocks)
                {
                    if (clocksToRegions.contains(clock))
                        clocksToRegions[clock].push_back(i);
                    else
                        clocksToRegions[clock] = { i };
                }
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


        // TODO: comment this function.
        std::vector<NetworkRegion> buildRegionGraphForeword();


        // Getters.
        [[nodiscard]] std::vector<NetworkRegion> getInitialRegions() const { return initialRegions; }
        [[nodiscard]] std::vector<int> getMaxConstants() const { return maxConstants; }


        [[nodiscard]] std::string toString() const
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

            // Display clocks to regions mapping.
            oss << "  Clocks to Regions: {";
            bool first = true;
            for (const auto &[clock, regions]: clocksToRegions)
            {
                if (!first)
                    oss << ", ";
                first = false;
                oss << clock << " ->";
                for (const auto &region: regions)
                    oss << " " << region;
            }
            oss << "}\n";

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
    };
}

#endif //TARZAN_RTSNETWORK_H
