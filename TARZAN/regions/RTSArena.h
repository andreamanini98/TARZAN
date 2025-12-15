#ifndef TARZAN_RTSARENA_H
#define TARZAN_RTSARENA_H

#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"


// A pointer to a region object.
using RegionPtr = const region::Region *;


namespace region
{
    class RTSArena : public RTS
    {
        absl::flat_hash_map<int, players_sym> locationsToPlayers{};


        /**
         * @brief Recursively extracts regions from a general CLTLoc formula while tracking nesting depth.
         *
         * This function processes general CLTLoc formulae which may contain pure formulae, unary operators (BOX, DIAMOND), or binary operators (UNTIL).
         * It recursively traverses the formula structure and generates regions for each pure subformula encountered.
         * The depth parameter tracks the level of nesting to enforce depth constraints.
         *
         * @param formula the general CLTLoc formula to process.
         * @param depth the current nesting depth (0 for top-level formula, incremented in recursive calls).
         * @return a vector containing unordered sets of regions, where:
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
        [[nodiscard]] std::vector<std::unordered_set<Region, RegionHash>> getRegionsFromGeneralCLTLocFormulaWithDepth(
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
         * @return an unordered set of regions satisfying the formula's location and clock constraints.
         */
        // TODO: fare check sugli invarianti prima di restituire le regioni.
        [[nodiscard]] std::unordered_set<Region, RegionHash> getRegionsFromPureCLTLocFormula(const cltloc::ast::pureCLTLocFormula &formula) const;


        /**
         * @brief Extracts regions from a general CLTLoc formula.
         *
         * This is a convenience wrapper around getRegionsFromGeneralCLTLocFormulaWithDepth() that initializes the depth to 0.
         * It processes general CLTLoc formulae and returns a vector of unordered sets of regions, with each unordered set corresponding to a pure subformula.
         * The result is a vector of unordered sets, since we want a set of regions for each pure CLTLoc formula in the general one.
         * For example:
         * - BOX phi produces one vector (regions for phi).
         * - DIAMOND phi produces one vector (regions for phi).
         * - phi UNTIL psi produces two vectors (regions for phi, then regions for psi).
         *
         * @param formula the general CLTLoc formula to process.
         * @return a vector containing unordered sets of regions, where each inner vector corresponds to a pure subformula.
         *
         * @throws NestedCLTLocFormulaException if nested formulae with depth > 0 are encountered.
         * @throws std::logic_error if an unhandled formula type is encountered.
         */
        [[nodiscard]] std::vector<std::unordered_set<Region, RegionHash>> getRegionsFromGeneralCLTLocFormula(
            const cltloc::ast::generalCLTLocFormula &formula) const;


        /// @warning Use only in comparisons with the parallel one.
        // TODO: fare check sugli invarianti prima di restituire le regioni.
        [[deprecated]] void omegaFilterSerial(const std::unordered_set<Region, RegionHash> &setG,
                                              const std::vector<RegionPtr> &toProcess,
                                              std::unordered_set<Region, RegionHash> &filteredRegions,
                                              std::vector<RegionPtr> &filteredRegionsPtr,
                                              const std::unordered_set<Region, RegionHash> &intersectionSet) const;


        /// @warning Use only in comparisons with the parallel one.
        // TODO: fare check sugli invarianti prima di restituire le regioni.
        [[deprecated]] void deltaFilterSerial(const std::unordered_set<Region, RegionHash> &setG,
                                              const std::vector<RegionPtr> &toProcess,
                                              std::unordered_set<Region, RegionHash> &filteredRegions,
                                              std::vector<RegionPtr> &filteredRegionsPtr,
                                              const std::unordered_set<Region, RegionHash> &intersectionSet) const;


        /**
         * @brief Applies omega filter for backward reachability in Timed Arenas.
         *
         * Iteratively processes regions from toProcess, computes their discrete predecessors, and adds valid predecessors to filteredRegions and filteredRegionsPtr.
         * A predecessor is valid if: (1) a transition with a unique action leads to setG, or (2) all transitions with a non-unique action lead to setG.
         *
         * @param setG set of goal regions.
         * @param toProcess vector of pointers to regions in setG that must be processed.
         * @param filteredRegions at the end of execution, will contain the filtered discrete predecessors regions ensuring the controller can reach setG.
         * @param filteredRegionsPtr at the end of execution, will contain the filtered discrete predecessors regions pointers ensuring the controller can reach setG.
         * @param intersectionSet for a region to be valid, it must also belong to intersectionSet.
         * @param skipPredecessorsInSetG if true, a predecessor already contained in setG is automatically considered valid.
         *
         * @warning The function updates filteredRegions and filteredRegionsPtr.
         * @warning toProcess and filteredRegionsPtr are vectors, since we may use OpenMP parallelization.
         */
        // TODO: fare check sugli invarianti prima di restituire le regioni.
        void omegaFilter(const std::unordered_set<Region, RegionHash> &setG,
                         const std::vector<RegionPtr> &toProcess,
                         std::unordered_set<Region, RegionHash> &filteredRegions,
                         std::vector<RegionPtr> &filteredRegionsPtr,
                         const std::unordered_set<Region, RegionHash> &intersectionSet,
                         bool skipPredecessorsInSetG) const;


        /**
         * @brief Applies delta filter for backward reachability in Timed Arenas.
         *
         * Iteratively processes regions from toProcess, computes their immediate delay predecessors, and adds valid predecessors to filteredRegions and filteredRegionsPtr.
         * A predecessor is valid if: (1) it is a controller region, or (2) it is an environment region and all its delay successors must lead to setG (not
         * restricted to immediate delay successors).
         *
         * @param setG set of goal regions.
         * @param toProcess vector of pointers to regions in setG that must be processed.
         * @param filteredRegions at the end of execution, will contain the filtered discrete predecessors regions ensuring the controller can reach setG.
         * @param filteredRegionsPtr at the end of execution, will contain the filtered discrete predecessors regions pointers ensuring the controller can reach setG.
         * @param intersectionSet for a region to be valid, it must also belong to intersectionSet.
         * @param skipPredecessorsInSetG if true, a predecessor already contained in setG is automatically considered valid.
         *
         * @warning The function updates filteredRegions and filteredRegionsPtr.
         * @warning toProcess and filteredRegionsPtr are vectors, since we may use OpenMP parallelization.
         */
        // TODO: fare check sugli invarianti prima di restituire le regioni.
        void deltaFilter(const std::unordered_set<Region, RegionHash> &setG,
                         const std::vector<RegionPtr> &toProcess,
                         std::unordered_set<Region, RegionHash> &filteredRegions,
                         std::vector<RegionPtr> &filteredRegionsPtr,
                         const std::unordered_set<Region, RegionHash> &intersectionSet,
                         bool skipPredecessorsInSetG) const;


        [[nodiscard]] std::string to_string() const;
    };
}

#endif //TARZAN_RTSARENA_H
