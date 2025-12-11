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
         * @brief Recursively extracts regions from a general CLTLoc formula while tracking nesting depth.
         *
         * This function processes general CLTLoc formulae which may contain pure formulae, unary operators (BOX, DIAMOND), or binary operators (UNTIL).
         * It recursively traverses the formula structure and generates regions for each pure subformula encountered.
         * The depth parameter tracks the level of nesting to enforce depth constraints.
         *
         * @param formula the general CLTLoc formula to process.
         * @param depth the current nesting depth (0 for top-level formula, incremented in recursive calls).
         * @return a vector containing vectors of regions, where:
         *         - Pure formulae produce one vector of regions.
         *         - Unary formulae (BOX, DIAMOND) produce one vector from their subformula.
         *         - Binary formulae (UNTIL) produce two vectors: left subformula regions followed by right subformula regions.
         *
         * @warning The admissible CLTLoc formulae are, up to now, constrained to depth at most 1.
         *          For example, BOX phi, DIAMOND phi, phi UNTIL psi, where phi and psi are pure CLTLoc formulae.
         *
         * @throws NestedCLTLocFormulaException if depth >= 1 when processing unary or binary operators (nested formulae not supported).
         * @throws std::logic_error if an unhandled formula type is encountered.
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
         * @brief Generates regions from a pure CLTLoc formula.
         *
         * Processes a pure CLTLoc formula (containing only location and clock constraints without temporal operators) and generates the corresponding set of
         * regions that satisfy those constraints. This is the base case for region extraction from CLTLoc formulae.
         *
         * @param formula the pure CLTLoc formula containing locations and clock constraints.
         * @return a vector of regions satisfying the formula's location and clock constraints.
         */
        [[nodiscard]] std::vector<Region> getRegionsFromPureCLTLocFormula(const cltloc::ast::pureCLTLocFormula &formula) const;


        /**
         * @brief Extracts regions from a general CLTLoc formula.
         *
         * This is a convenience wrapper around getRegionsFromGeneralCLTLocFormulaWithDepth() that initializes the depth to 0.
         * It processes general CLTLoc formulae and returns a vector of region vectors, with each vector corresponding to a pure subformula.
         * The result is a vector of vectors, since we want a set of regions for each pure CLTLoc formula in the general one.
         * For example:
         * - BOX phi produces one vector (regions for phi).
         * - DIAMOND phi produces one vector (regions for phi).
         * - phi UNTIL psi produces two vectors (regions for phi, then regions for psi).
         *
         * @param formula the general CLTLoc formula to process.
         * @return a vector containing vectors of regions, where each inner vector corresponds to a pure subformula.
         *
         * @throws NestedCLTLocFormulaException if nested formulae with depth > 0 are encountered.
         * @throws std::logic_error if an unhandled formula type is encountered.
         */
        [[nodiscard]] std::vector<std::vector<Region>> getRegionsFromGeneralCLTLocFormula(const cltloc::ast::generalCLTLocFormula &formula) const;


        [[nodiscard]] std::string to_string() const;
    };
}

#endif //TARZAN_RTSARENA_H
