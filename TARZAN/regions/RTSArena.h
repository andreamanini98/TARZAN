#ifndef TARZAN_RTSARENA_H
#define TARZAN_RTSARENA_H

#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"


namespace region
{
    class RTSArena : public RTS
    {
        absl::flat_hash_map<int, char> locationsToPlayers{};


        /**
         * @brief
         *
         * @param formula
         * @param depth
         * @return
         *
         * @warning The admissible CLTLoc formulae are, up tp noe, constrained to depth at most 1 (e.g., BOX phi, DIAMOND phi, phi UNTIL psi, where phi and
         *          psi are pure CLTLoc formulae).
         *
         * @throws NestedCLTLocFormulaException
         * @throws std::logic_error
         */
        [[nodiscard]] std::vector<std::vector<Region>> getRegionsFromGeneralCLTLocFormulaWithDepth(
            const cltloc::ast::generalCLTLocFormula &formula, int depth) const;


    public:
        explicit RTSArena(const timed_automaton::ast::timedArena &arena)
        {
            clocksIndices = arena.getClocksIndices();
            locationsToInt = arena.mapLocationsToInt();
            maxConstants = arena.getMaxConstants(clocksIndices);
            initialLocations = arena.getInitialLocations(locationsToInt);
            outTransitions = arena.getOutTransitions(locationsToInt);
            inTransitions = arena.getInTransitions(locationsToInt);
            invariants = arena.getInvariants(locationsToInt);
            urgentLocations = arena.getUrgentLocations(locationsToInt);
            locationsToPlayers = arena.mapLocationsToPlayers(locationsToInt);

            const int numOfClocks = static_cast<int>(clocksIndices.size());
            const auto &variables = arena.getVariables();

            for (const int loc: initialLocations)
                initialRegions.emplace_back(numOfClocks, loc, variables);
        }


        explicit RTSArena(const timed_automaton::ast::timedArena &arena, const cltloc::ast::generalCLTLocFormula &formula)
        {
            clocksIndices = arena.getClocksIndices();
            locationsToInt = arena.mapLocationsToInt();
            maxConstants = arena.getMaxConstants(clocksIndices, formula);
            initialLocations = arena.getInitialLocations(locationsToInt);
            outTransitions = arena.getOutTransitions(locationsToInt);
            inTransitions = arena.getInTransitions(locationsToInt);
            invariants = arena.getInvariants(locationsToInt);
            urgentLocations = arena.getUrgentLocations(locationsToInt);
            locationsToPlayers = arena.mapLocationsToPlayers(locationsToInt);

            const int numOfClocks = static_cast<int>(clocksIndices.size());
            const auto &variables = arena.getVariables();

            for (const int loc: initialLocations)
                initialRegions.emplace_back(numOfClocks, loc, variables);
        }


        /**
         * @brief
         *
         * @param formula
         * @return
         */
        [[nodiscard]] std::vector<Region> getRegionsFromPureCLTLocFormula(const cltloc::ast::pureCLTLocFormula &formula) const;


        /**
         * @brief
         *
         * The result is a vector of vectors, since we want a set of regions for each pure CLTLoc formula in the general one.
         *
         * @param formula
         * @return
         */
        [[nodiscard]] std::vector<std::vector<Region>> getRegionsFromGeneralCLTLocFormula(const cltloc::ast::generalCLTLocFormula &formula) const;


        [[nodiscard]] std::string to_string() const;
    };
}

#endif //TARZAN_RTSARENA_H
