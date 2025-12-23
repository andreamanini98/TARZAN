#ifndef TARZAN_RTSARENA_H
#define TARZAN_RTSARENA_H

#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"


// A pointer to a region object.
using RegionPtr = const region::Region *;
using regionSet = std::unordered_set<region::Region, region::RegionHash>;


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
         *
         * @warning Returned regions satisfy the invariants of the underlying Timed Arena.
         */
        [[nodiscard]] inline std::vector<regionSet> getRegionsFromGeneralCLTLocFormulaWithDepth(const cltloc::ast::generalCLTLocFormula &formula,
                                                                                                int depth) const;


        /**
         * @brief Used to determine whether a region must be ignored by omega and delta filters.
         *
         * @param reg the region to check.
         * @param setG set of goal regions.
         * @param intersectionSet for a region to be valid, it must also belong to intersectionSet.
         * @param skipPredecessorsInSetG if true, a predecessor already contained in setG is automatically considered valid.
         * @return true if the region must be ignored, false otherwise.
         */
        [[nodiscard]] inline bool skipRegion(const Region &reg, const regionSet &setG, const regionSet &intersectionSet, bool skipPredecessorsInSetG) const;


        /**
         * @brief Used to merge results before returning regions in omega and delta filters.
         *
         * @param threadLocalRegions a vector containing the regions to be merged.
         * @param filteredRegions at the end of execution, will contain the filtered discrete predecessors regions ensuring the controller can reach setG.
         *
         * @warning The function updates filteredRegions.
         */
        static inline void mergeResults(const std::vector<std::vector<Region>> &threadLocalRegions, regionSet &filteredRegions);


        /**
         * @brief Solves a game where the formula is: BOX phi.
         *
         * @param unaryFormula the formula to solve.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if the formula has the wrong size.
         */
        [[nodiscard]] inline bool solveGameWithBoxFormula(const cltloc::ast::unaryCLTLocFormula &unaryFormula) const;


        /**
         * @brief Solves a game where the formula is: DIAMOND phi.
         *
         * @param unaryFormula the formula to solve.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if the formula has the wrong size.
         */
        [[nodiscard]] inline bool solveGameWithDiamondFormula(const cltloc::ast::unaryCLTLocFormula &unaryFormula) const;


        /**
         * @brief Solves a game where the formula is: phi UNTIL psi.
         *
         * @param binaryFormula the formula to solve.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if the formula has the wrong size.
         */
        [[nodiscard]] inline bool solveGameWithUntilFormula(const cltloc::ast::binaryCLTLocFormula &binaryFormula) const;


        /**
         * @brief Solves a game where the formula is: NEXT (phi UNTIL psi).
         *
         * @param formula the formula to solve.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if the formula is not of the form: NEXT (phi UNTIL psi).
         */
        [[nodiscard]] inline bool solveGameWithNextFormula(const cltloc::ast::generalCLTLocFormula &formula) const;


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
         *
         * @warning Returned regions satisfy the invariants of the underlying Timed Arena.
         */
        [[nodiscard]] regionSet getRegionsFromPureCLTLocFormula(const cltloc::ast::pureCLTLocFormula &formula) const;


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
         *
         * @warning Returned regions satisfy the invariants of the underlying Timed Arena.
         */
        [[nodiscard]] std::vector<regionSet> getRegionsFromGeneralCLTLocFormula(const cltloc::ast::generalCLTLocFormula &formula) const;


        /**
         * @brief Applies omega filter for backward reachability in Timed Arenas.
         *
         * Iteratively processes regions from toProcess, computes their discrete predecessors, and adds valid predecessors to filteredRegions.
         * A predecessor is valid if: (1) a transition with a unique action leads to setG, or (2) all transitions with a non-unique action lead to setG.
         *
         * @param setG set of goal regions.
         * @param toProcess vector of pointers to regions in setG that must be processed.
         * @param filteredRegions at the end of execution, will contain the filtered discrete predecessors regions ensuring the controller can reach setG.
         * @param intersectionSet for a region to be valid, it must also belong to intersectionSet.
         * @param skipPredecessorsInSetG if true, a predecessor already contained in setG is automatically considered valid.
         * @param checkAllSuccessorsInvariants if true, all delay successors of environment regions must additionally satisfy invariants (useful for safety).
         *
         * @warning The function updates filteredRegions.
         * @warning toProcess is a vector, since we may use OpenMP parallelization.
         */
        void omegaFilter(const regionSet &setG,
                         const std::vector<RegionPtr> &toProcess,
                         regionSet &filteredRegions,
                         const regionSet &intersectionSet,
                         bool skipPredecessorsInSetG,
                         bool checkAllSuccessorsInvariants) const;


        /**
         * @brief Applies delta filter for backward reachability in Timed Arenas.
         *
         * Iteratively processes regions from toProcess, computes their immediate delay predecessors, and adds valid predecessors to filteredRegions.
         * A predecessor is valid if: (1) it is a controller region, or (2) it is an environment region and all its delay successors must lead to setG (not
         * restricted to immediate delay successors).
         *
         * @param setG set of goal regions.
         * @param toProcess vector of pointers to regions in setG that must be processed.
         * @param filteredRegions at the end of execution, will contain the filtered discrete predecessors regions ensuring the controller can reach setG.
         * @param intersectionSet for a region to be valid, it must also belong to intersectionSet.
         * @param skipPredecessorsInSetG if true, a predecessor already contained in setG is automatically considered valid.
         * @param checkAllSuccessorsInvariants if true, all delay successors of environment regions must additionally satisfy invariants (useful for safety).
         *
         * @warning The function updates filteredRegions.
         * @warning toProcess is a vector, since we may use OpenMP parallelization.
         */
        void deltaFilter(const regionSet &setG,
                         const std::vector<RegionPtr> &toProcess,
                         regionSet &filteredRegions,
                         const regionSet &intersectionSet,
                         bool skipPredecessorsInSetG,
                         bool checkAllSuccessorsInvariants) const;


        /**
         * @brief Determines whether the controller wins in a timed reachability game where the winning condition is: DIAMOND phi.
         *
         * @param setG set of goal regions.
         * @param toProcess vector of pointers to regions in setG that must be processed.
         * @param maxIter the maximum number of iterations to perform; if reached, terminate even if the fixpoint has not been reached.
         * @return true if the controller wins, false otherwise.
         *
         * @warning The function updates setG and toProcess.
         */
        [[nodiscard]] bool timedReachability(regionSet &setG, std::vector<RegionPtr> &toProcess, int maxIter) const;


        /**
         * @brief Determines whether the controller wins in a timed reachability game where the winning condition is: phi UNTIL psi.
         *
         * @param setPhi for regions to be valid, they must also be contained in this set.
         * @param setG set of goal regions.
         * @param toProcess vector of pointers to regions in setG that must be processed.
         * @param maxIter the maximum number of iterations to perform; if reached, terminate even if the fixpoint has not been reached.
         * @return true if the controller wins, false otherwise.
         *
         * @warning The function updates setG and toProcess.
         */
        [[nodiscard]] bool timedReachability(const regionSet &setPhi, regionSet &setG, std::vector<RegionPtr> &toProcess, int maxIter) const;


        /**
         * @brief Determines whether the controller wins in a timed reachability game where the winning condition is: NEXT (phi UNTIL psi).
         *
         * @param setPhi for regions to be valid, they must also be contained in this set.
         * @param setG set of goal regions.
         * @param toProcess vector of pointers to regions in setG that must be processed.
         * @param maxIter the maximum number of iterations to perform; if reached, terminate even if the fixpoint has not been reached.
         * @return true if the controller wins, false otherwise.
         *
         * @warning The function updates setG and toProcess.
         */
        [[nodiscard]] bool timedNextReachability(const regionSet &setPhi, regionSet &setG, std::vector<RegionPtr> &toProcess, int maxIter) const;


        /**
         * @brief Determines whether the controller wins in a timed reachability game where the winning condition is: BOX phi.
         *
         * @param setG set of goal regions.
         * @param toProcess vector of pointers to regions in setG that must be processed.
         * @param maxIter the maximum number of iterations to perform; if reached, terminate even if the fixpoint has not been reached.
         * @return true if the controller wins, false otherwise.
         *
         * @warning The function updates setG and toProcess.
         */
        [[nodiscard]] bool timedSafety(regionSet &setG, std::vector<RegionPtr> &toProcess, int maxIter) const;


        /**
         * @brief Determines whether the controller wins a Timed CLTLoc Game given the general CLTLoc formula.
         *
         * Currently, it only detects the nature of the given formula without recursion.
         * The only allowed recursion is of formulae like NEXT (phi UNTIL psi).
         * Supported formulae are: BOX phi, DIAMOND phi, phi UNTIL psi, NEXT (phi UNTIL psi).
         *
         * @param formula the general CLTLoc formula to process.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if an unhandled formula type is encountered or a pure formula is given as the parameter value.
         */
        [[nodiscard]] bool solveTimedCLTLocGame(const cltloc::ast::generalCLTLocFormula &formula) const;


        [[nodiscard]] std::string to_string() const;
    };
}

#endif //TARZAN_RTSARENA_H
