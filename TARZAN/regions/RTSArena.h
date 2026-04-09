#ifndef TARZAN_RTSARENA_H
#define TARZAN_RTSARENA_H

#include "TARZAN/regions/Region.h"
#include "TARZAN/regions/RTS.h"
#include "TARZAN/regions/StrategyGraph.h"


// A pointer to a region object.
using RegionPtr = const region::Region *;
using regionSet = std::unordered_set<region::Region, region::RegionHash>;


namespace region
{
    class RTSArena : public RTS
    {
        absl::flat_hash_map<int, players_sym> locationsToPlayers{};

        // Whenever set to true, the function piFilter() will compute the strategy graph for a given application of pi.
        bool computeStrategyGraph;

        StrategyGraph strategyGraph{};


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
         * @brief Used to determine whether piFilter must ignore a region.
         *
         * @param reg the region to check.
         * @param setG set of goal regions.
         * @param intersectionSet for a region to be valid, it must also belong to intersectionSet.
         * @param skipPredecessorsInSetG if true, a predecessor already contained in setG is automatically considered valid.
         * @return true if the region must be ignored, false otherwise.
         */
        [[nodiscard]] inline bool skipRegion(const Region &reg, const regionSet &setG, const regionSet &intersectionSet, bool skipPredecessorsInSetG) const;


        /**
         * @brief Used to merge results before returning regions in piFilter.
         *
         * @param threadLocalRegions a vector containing the regions to be merged.
         * @param filteredRegions at the end of execution, will contain the filtered discrete predecessors regions ensuring the controller can reach setG.
         *
         * @warning The function updates filteredRegions.
         */
        static inline void mergeResults(const std::vector<std::vector<Region>> &threadLocalRegions, regionSet &filteredRegions);


        /**
         * @brief Computes a map in which keys are action names and values are Booleans set to true if all outgoing transitions
         *        from reg with such action end in a region in setG; the Boolean values are set to false otherwise.
         *
         * @param reg the region from which all outgoing transitions must end in setG for their action to be inserted with value true.
         * @param setG set of goal regions.
         * @return a map between actions and Booleans.
         */
        [[nodiscard]] inline absl::flat_hash_map<std::string, bool> computeValidActions(const Region &reg, const regionSet &setG) const;


        /**
         * @brief Determines whether all output transitions from reg over action 'action' lead to a discrete successor in setG.
         *
         * @param reg the region from which all outgoing transitions must end in setG for action 'action'.
         * @param setG set of goal regions.
         * @param action the action used to determine which transitions are considered.
         * @param isRegionValid at the end of execution, true if all output transitions from reg over action 'action' lead to a discrete successor in setG; false otherwise.
         * @param atLeastOneDiscreteSuccessor at the end of execution it must be true: at least one discrete successor must be computed for the game to be
         *                                    non-blocking (which is our assumption when creating arenas and games).
         *
         * @warning The function updates isRegionValid.
         * @warning The function updates atLeastOneDiscreteSuccessor.
         */
        inline void everyOutTransitionIsInSetG(const Region &reg,
                                               const regionSet &setG,
                                               const std::string &action,
                                               bool &isRegionValid,
                                               bool &atLeastOneDiscreteSuccessor) const;


        /**
         * @brief Determines whether an environment region satisfies the pi_e condition as detailed in our paper.
         *
         * @param reg the region over which the pi_e condition must be evaluated.
         * @param setG set of goal regions.
         * @param validActions actions that the controller can potentially choose to declare a move guaranteeing it to reach a region in setG.
         * @param checkAllSuccessorsInvariants if true, all delay successors of environment regions must additionally satisfy invariants (useful for safety).
         * @return true if reg satisfies the pi_e condition, false otherwise.
         */
        [[nodiscard]] inline bool piEnvironment(const Region &reg,
                                                const regionSet &setG,
                                                const absl::flat_hash_map<std::string, bool> &validActions,
                                                bool checkAllSuccessorsInvariants) const;


        /**
         * @brief Determines whether a controller region satisfies the pi_c condition as detailed in our paper.
         *
         * @param validActions actions that the controller can potentially choose to declare a move guaranteeing it to reach a region in setG.
         * @return true if reg satisfies the pi_c condition, false otherwise.
         */
        [[nodiscard]] static inline bool piController(const absl::flat_hash_map<std::string, bool> &validActions);


        /**
         * @brief Auxiliary function used to execute piEnvironment or piController based on whether the region is of the environment or the controller, respectively.
         *
         * @param reg the region over which the pi_e or pi_c condition must be evaluated.
         * @param setG set of goal regions.
         * @param threadLocalRegions vector of vectors collecting the resulting region if valid.
         *                           Each external vector corresponds to a thread in OpenMP (only one inner vector is present if OpenMP is not enabled).
         * @param validActions actions that the controller can potentially choose to declare a move guaranteeing it to reach a region in setG.
         * @param checkAllSuccessorsInvariants if true, all delay successors of environment regions must additionally satisfy invariants (useful for safety).
         */
        inline void collectLegalRegionByPi(const Region &reg,
                                           const regionSet &setG,
                                           std::vector<std::vector<Region>> &threadLocalRegions,
                                           const absl::flat_hash_map<std::string, bool> &validActions,
                                           bool checkAllSuccessorsInvariants) const;


        /**
         * @brief Determines the transitions inside the strategy graph.
         *
         * Whenever a region, which can be the source of a move, is collected by the backward algorithm, this function adds the respective move transition into
         * the strategy graph, i.e., a transition in the strategy graph consists in a delay and a discrete transition together, which is exactly the definition of TCG moves.
         *
         * @param sourceRegion the source region of a move.
         * @param arenaTransition the original Timed ARena transitino over which the strategy transition is computed.
         * @param targetRegion the target region of a move.
         * @param cv the clock valuation of a discrete predecessor of targetRegion.
         * @param setG set of goal regions.
         * @param threadLocalRegions vector of vectors collecting the resulting region if valid.
         *                           Each external vector corresponds to a thread in OpenMP (only one inner vector is present if OpenMP is not enabled).
         * @param validActions actions that the controller can potentially choose to declare a move guaranteeing it to reach a region in setG.
         * @param checkAllSuccessorsInvariants if true, all delay successors of environment regions must additionally satisfy invariants (useful for safety).
         *
         * @note The delay is not considered here. It may be derived from the stored regions in the strategy graph.
         */
        inline void collectLegalRegionByPiStrategy(const Region &sourceRegion,
                                                   const transition &arenaTransition,
                                                   const Region &targetRegion,
                                                   const clockValuation &cv,
                                                   const regionSet &setG,
                                                   std::vector<std::vector<Region>> &threadLocalRegions,
                                                   const absl::flat_hash_map<std::string, bool> &validActions,
                                                   bool checkAllSuccessorsInvariants);


        /**
         * @brief Solves a game where the formula is: BOX phi.
         *
         * @param unaryFormula the formula to solve.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if the formula has the wrong size.
         */
        [[nodiscard]] inline bool solveGameWithBoxFormula(const cltloc::ast::unaryCLTLocFormula &unaryFormula);


        /**
         * @brief Solves a game where the formula is: DIAMOND phi.
         *
         * @param unaryFormula the formula to solve.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if the formula has the wrong size.
         */
        [[nodiscard]] inline bool solveGameWithDiamondFormula(const cltloc::ast::unaryCLTLocFormula &unaryFormula);


        /**
         * @brief Solves a game where the formula is: phi UNTIL psi.
         *
         * @param binaryFormula the formula to solve.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if the formula has the wrong size.
         */
        [[nodiscard]] inline bool solveGameWithUntilFormula(const cltloc::ast::binaryCLTLocFormula &binaryFormula);


        /**
         * @brief Solves a game where the formula is: NEXT (phi UNTIL psi).
         *
         * @param formula the formula to solve.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if the formula is not of the form: NEXT (phi UNTIL psi).
         */
        [[nodiscard]] inline bool solveGameWithNextFormula(const cltloc::ast::generalCLTLocFormula &formula);


        /**
         * @brief Solves a game where the formula is: NEXT^{n_0} phi_0 && NEXT^{n_1} phi_1 && ... && NEXT^{n_k} phi_k, where n_i < n_{i+1}.
         *
         * @param formulae the conjunction of formulae to solve.
         * @return true if the controller wins, false otherwise.
         *
         * @warning Remember to specify the NEXT operator with applicationCount equal to 0 if you need a formula to be true at the current position in time.
         */
        [[nodiscard]] inline bool solveGameWithAndNextConjunction(const std::vector<cltloc::ast::generalCLTLocFormula> &formulae);


    public:
        RTSArena(const timed_automaton::ast::timedArena &arena, const bool computeStrategyGraph) : computeStrategyGraph(computeStrategyGraph)
        {
            clocksIndices = arena.getClocksIndices();
            locationsToInt = arena.mapLocationsToInt();
            intToLocations = arena.mapIntToLocations();
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


        RTSArena(const timed_automaton::ast::timedArena &arena,
                 const cltloc::ast::generalCLTLocFormula &formula,
                 const bool computeStrategyGraph) : computeStrategyGraph(computeStrategyGraph)
        {
            clocksIndices = arena.getClocksIndices();
            locationsToInt = arena.mapLocationsToInt();
            intToLocations = arena.mapIntToLocations();
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


        RTSArena(const timed_automaton::ast::timedArena &arena,
                 const cltloc::ast::conjunctionOfFormulae &conjunction,
                 const bool computeStrategyGraph) : computeStrategyGraph(computeStrategyGraph)
        {
            clocksIndices = arena.getClocksIndices();
            locationsToInt = arena.mapLocationsToInt();
            intToLocations = arena.mapIntToLocations();
            maxConstants = arena.getMaxConstants(clocksIndices, conjunction);
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
         * @brief Computes the regions from which there exists a move that guarantees the controller to reach a region in setG.
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
        void piFilter(const regionSet &setG,
                      const std::vector<RegionPtr> &toProcess,
                      regionSet &filteredRegions,
                      const regionSet &intersectionSet,
                      bool skipPredecessorsInSetG,
                      bool checkAllSuccessorsInvariants);


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
        [[nodiscard]] bool timedReachability(regionSet &setG, std::vector<RegionPtr> &toProcess, int maxIter);


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
        [[nodiscard]] bool timedReachability(const regionSet &setPhi, regionSet &setG, std::vector<RegionPtr> &toProcess, int maxIter);


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
        [[nodiscard]] bool timedNextReachability(const regionSet &setPhi, regionSet &setG, std::vector<RegionPtr> &toProcess, int maxIter);


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
        [[nodiscard]] bool timedSafety(regionSet &setG, std::vector<RegionPtr> &toProcess, int maxIter);


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
         * @throws std::logic_error if an unhandled formula type is encountered, or a pure formula is given as the parameter value.
         */
        [[nodiscard]] bool solveTimedCLTLocGame(const cltloc::ast::generalCLTLocFormula &formula);


        /**
         * @brief Determines whether the controller wins a Timed CLTLoc Game given a conjunction of general CLTLoc formulae.
         *
         * Supported formulae are:
         * AND_NEXT NEXT^{n_0} phi_0 && NEXT^{n_1} phi_1 && ... && NEXT^{n_k} phi_k, where n_i < n_{i+1}.
         *
         * @param conjunction the conjunction of general CLTLoc formulae to process.
         * @return true if the controller wins, false otherwise.
         *
         * @throws std::logic_error if an unhandled conjunction type is encountered.
         *
         * @warning Remember to specify the NEXT operator with applicationCount equal to 0 if you need a formula to be true at the current position in time.
         */
        [[nodiscard]] bool solveTimedCLTLocGame(const cltloc::ast::conjunctionOfFormulae &conjunction);


        // TODO: Implementare qui una funzione per esplorare o sintetizzare una strategia. Farlo magari in base al tipo di gioco considerato (winning condition).

        // TODO: durante l'esplorazione o la sintesi, fare dei print fatti bene delle regioni, ossia nome della regione e valore dei clock.
        //       vedere se è anche possibile derivare il delay che deve scegliere il controller quando si trova nelle controller regions.

        // TODO: vedere se si riesce a fare sintesi anche con i next until.

        // TODO: prossimi passi:
        //       1) Fare algoritmo che esplora lo strategy graph ed estrae una strategia.
        //       2) Fare lo strategy graph per la safety e next until.
        //       3) Stampare a schermo in maniera carina le strategie. Okkio magari a differenziare regioni del controller e dell'environment.
        //       Occorre determinare quali clock sono resettati in una transizione dell'arena, così da dirlo anche nelle strategie??
        //       I delay che devono essere presi diventano banali se si assume di avere un clock now che tiene traccia del tempo trascorso in ogni mossa.

        [[nodiscard]] std::string strategyGraphToString() const
        {
            return strategyGraph.to_string(intToLocations);
        }


        [[nodiscard]] std::string to_string() const;
    };
}

#endif //TARZAN_RTSARENA_H
