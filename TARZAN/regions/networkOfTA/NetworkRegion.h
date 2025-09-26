#ifndef TARZAN_NETWORKREGION_H
#define TARZAN_NETWORKREGION_H

#include <vector>
#include "absl/container/flat_hash_map.h"

#include "TARZAN/regions/Region.h"

// An advantage of defining the configuration of the network using a vector of regions is that, when computing discrete successors, you can compute
// them only on the necessary regions, instead of computing the discrete successor of the whole cartesian product region.

namespace networkOfTA
{
    class NetworkRegion
    {
        /// Each region corresponds to a Timed Automaton of the network.
        std::vector<region::Region> regions{};

        /// Contains the indices of the regions vector such that the corresponding region is either of class A or class C.
        std::vector<int> isAorC{};

        /**
         * Keeps track of the clocks ordering between different regions. Key = region index.
         * The maps in the front capture the smallest equivalent fractional parts, while the maps in the back the highest equivalent fractional parts.
         */
        std::deque<absl::flat_hash_map<int, boost::dynamic_bitset<>>> isB{};


    public:
        explicit NetworkRegion(const std::vector<region::Region> &regions, const bool allRegionsAreInitial) : regions(regions)
        {
            // If every region is initial, they must all belong to class A.
            if (allRegionsAreInitial)
            {
                const int isAorCSize = static_cast<int>(regions.size());
                isAorC.resize(isAorCSize);

                // We insert values from 0 to isAorCSize - 1 since every region is initial.
                std::iota(isAorC.begin(), isAorC.end(), 0);
            }
        }


        // TODO: andranno anche implementate le stesse funzioni hash per vedere se una regione è già stata calcolata o no (ovviamente adattando il caso alle network).


        /**
         * @brief Computes the immediate delay successor of the current network region.
         *
         * @param maxConstants the maximum constants of the Timed Automata from which the network region is derived.
         * @return a NetworkRegion immediate delay successor of the current network region.
         */
        [[nodiscard]] NetworkRegion getImmediateDelaySuccessor(const std::vector<int> &maxConstants) const;


        /**
         * @brief
         *
         * @param transitions
         * @param clockIndices
         * @param locationsToInt
         * @return
         */
        // TODO: potrebbe servirti come parametro anche absl::flat_hash_map<std::string, std::vector<int>> clocksToRegions{}; (glielo darai poi dal RTSNetwork)
        [[nodiscard]] std::vector<NetworkRegion> getImmediateDiscreteSuccessors(const std::vector<transition> &transitions,
                                                                                const std::vector<std::unordered_map<std::string, int>> &clockIndices,
                                                                                const std::vector<std::unordered_map<std::string, int>> &locationsToInt) const;


        /**
         * @brief Creates a deep copy of this region.
         *
         * @return a new NetworkRegion object that is a copy of this one.
         */
        [[nodiscard]] NetworkRegion clone() const
        {
            return { *this };
        }


        [[nodiscard]] std::string toString() const
        {
            std::ostringstream oss;

            oss << "NetworkRegion {\n";
            oss << "  Regions (" << regions.size() << "):\n";

            for (size_t i = 0; i < regions.size(); i++)
            {
                // Indent the Region's toString output.
                std::istringstream regionStream(regions[i].toString());
                std::string line;
                oss << "  [" << i << "]:\n";

                while (std::getline(regionStream, line))
                    if (!line.empty())
                        oss << "    " << line << "\n";
            }

            oss << "  isAorC (" << isAorC.size() << "): [";
            for (size_t i = 0; i < isAorC.size(); i++)
            {
                oss << isAorC[i];
                if (i < isAorC.size() - 1)
                    oss << ", ";
            }
            oss << "]\n";

            oss << "  isB (" << isB.size() << "):\n";
            int index = 0;
            for (const auto &map: isB)
            {
                oss << "    [" << index++ << "]: {";
                bool first = true;
                for (const auto &[key, bitset]: map)
                {
                    if (!first)
                        oss << ", ";
                    first = false;
                    oss << key << " -> " << bitset;
                }
                oss << "}\n";
            }

            oss << "}\n";
            return oss.str();
        }
    };
}


#endif //TARZAN_NETWORKREGION_H
