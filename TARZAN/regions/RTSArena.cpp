#include "RTSArena.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/exceptions/nestedCLTLocFormula_exception.h"
#include "TARZAN/exceptions/cannotSynthesizeStrategies_exception.h"
#include "TARZAN/utilities/function_utilities.h"
#include <cstdio>
#include "TARZAN/utilities/printing_utilities.h"

#include <iostream>
#include <numeric>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef _TBB
#include "oneapi/tbb/parallel_for.h"
#include "tbb/enumerable_thread_specific.h"
#endif

#define DEBUG_MEMORY 1

// #define RTSARENA_DEBUG
#define THROW_NESTEDCLTLOC_EXCEPTION

// Threshold for enabling parallel execution (derived from manual experiments).
#define PARALLEL_THRESHOLD 400

// Total number of iterations performed by fixed point algorithms.
#define MAX_ITERATIONS 10000


std::unordered_map<int, std::string> region::RTSArena::getIndicesToClocksMap() const
{
    std::unordered_map<int, std::string> indicesToClocks;
    indicesToClocks.reserve(clocksIndices.size());

    for (const auto &[name, index]: clocksIndices)
        indicesToClocks.emplace(index, name);

    return indicesToClocks;
}


regionSet region::RTSArena::getRegionsFromPureCLTLocFormula(const cltloc::ast::pureCLTLocFormula &formula) const
{
#ifdef RTSARENA_DEBUG

    std::cout << "Now handling the following pure CLTLoc formula: " << formula << std::endl;

#endif

    regionSet res{};

    const int nClocks = static_cast<int>(clocksIndices.size());

    for (const auto &[locations, clockConstraints]: formula.disjuncts)
        res.merge(Region::generateRegionsFromConstraints(locations, clockConstraints, clocksIndices, locationsToInt, maxConstants, nClocks));

    if (!invariants.empty())
    {
        // Removing regions that do not satisfy the invariants of the Timed Arena.
        std::erase_if(res, [this](const Region &reg)
        {
            if (const auto it = invariants.find(reg.getLocation()); it != invariants.end())
                return !isInvariantSatisfied(it->second, reg.getClockValuation(), clocksIndices);
            return false; // No invariant = keep the region.
        });
    }

    return res;
}


// TODO: per ora l'implementazione corrente va bene perchè non hai annidamenti e quindi hai al più due vettori nel vettore esterno risultante.
//       Vedere di trovare il modo di rendere il tutto più generale qualora vengano resi disponibili livelli di annidamento nelle formule.
inline std::vector<regionSet> region::RTSArena::getRegionsFromGeneralCLTLocFormulaWithDepth(const cltloc::ast::generalCLTLocFormula &formula,
                                                                                            int depth) const // NOLINT
{
    std::vector<regionSet> res{};

    boost::apply_visitor([this, &res, depth]<typename T0>(T0 const &val) {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::pureCLTLocFormula>>)
        {
#ifdef RTSARENA_DEBUG

            std::cout << "Calling getRegionsFromGeneralCLTLocFormulaWithDepth from pure case" << std::endl;

#endif

            // Base case - pure formula: compute regions from this formula.
            const auto &pureFormula = val.get();
            res.push_back(getRegionsFromPureCLTLocFormula(pureFormula));
            // ---
        } else if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::unaryCLTLocFormula>>)
        {
#ifdef RTSARENA_DEBUG

            std::cout << "Calling getRegionsFromGeneralCLTLocFormulaWithDepth from unary case" << std::endl;

#endif

#ifdef THROW_NESTEDCLTLOC_EXCEPTION

            if (depth >= 1)
                throw NestedCLTLocFormulaException("Nested formulae with depth > 0 are not supported.");

#endif

            // Recursive case - unary formula.
            const auto &unaryFormula = val.get();

            std::vector<regionSet> tmp = getRegionsFromGeneralCLTLocFormulaWithDepth(unaryFormula.rightFormula, depth + 1);
            res.reserve(res.size() + tmp.size());
            res.insert(res.end(), std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
            // ---
        } else if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::binaryCLTLocFormula>>)
        {
#ifdef RTSARENA_DEBUG

            std::cout << "Calling getRegionsFromGeneralCLTLocFormulaWithDepth from binary case" << std::endl;

#endif

#ifdef THROW_NESTEDCLTLOC_EXCEPTION

            if (depth >= 1)
                throw NestedCLTLocFormulaException("Nested formulae with depth > 0 are not supported.");

#endif

            // Recursive case - binary formula.
            const auto &binaryFormula = val.get();

            std::vector<regionSet> leftTmp = getRegionsFromGeneralCLTLocFormulaWithDepth(binaryFormula.leftFormula, depth + 1);
            res.reserve(res.size() + leftTmp.size());
            res.insert(res.end(), std::make_move_iterator(leftTmp.begin()), std::make_move_iterator(leftTmp.end()));

            std::vector<regionSet> rightTmp = getRegionsFromGeneralCLTLocFormulaWithDepth(binaryFormula.rightFormula, depth + 1);
            res.reserve(res.size() + rightTmp.size());
            res.insert(res.end(), std::make_move_iterator(rightTmp.begin()), std::make_move_iterator(rightTmp.end()));
        } else
            throw std::logic_error("Unhandled formula type in getRegionsFromGeneralCLTLocFormulaWithDepth.");
    }, formula.value);

    return res;
}


std::vector<regionSet> region::RTSArena::getRegionsFromGeneralCLTLocFormula(const cltloc::ast::generalCLTLocFormula &formula) const
{
    return getRegionsFromGeneralCLTLocFormulaWithDepth(formula, 0);
}


inline bool region::RTSArena::skipRegion(const Region &reg, const regionSet &setG, const regionSet &intersectionSet, const bool skipPredecessorsInSetG) const
{
    // If skipPredecessorsInSetG is true and the predecessor is already in setG, skip it.
    if (skipPredecessorsInSetG && setG.contains(reg))
        return true;

    // If a region does not belong to the intersection set, we do not insert it into filteredRegions.
    if (!intersectionSet.empty() && !intersectionSet.contains(reg))
        return true;

    // For a predecessor to be valid, it must satisfy the invariants.
    if (!invariants.empty())
    {
        if (const auto it = invariants.find(reg.getLocation()); it != invariants.end())
            if (!isInvariantSatisfied(it->second, reg.getClockValuation(), clocksIndices))
                return true;
    }

    return false;
}


inline void region::RTSArena::mergeResults(const std::vector<std::vector<Region>> &threadLocalRegions, regionSet &filteredRegions)
{
    // Sequential merge of thread-local results.
    for (const auto &localRegions: threadLocalRegions)
        for (const auto &reg: localRegions)
            filteredRegions.insert(reg);
}


inline absl::flat_hash_map<std::string, bool> region::RTSArena::computeValidActions(const Region &reg, const regionSet &setG) const
{
    // Map between action names and a Boolean.
    // The Boolean is set to true if every transition with action in validActions generates a discrete successor in setG; the Boolean is false otherwise.
    absl::flat_hash_map<std::string, bool> validActions{};

    for (const auto &outTrans: outTransitions[reg.getLocation()])
    {
        const std::vector<Region> discreteSuccessors = reg.getImmediateDiscreteSuccessors({ outTrans }, clocksIndices, locationsToInt);

        // Skip transitions that don't produce valid successors.
        if (discreteSuccessors.empty())
            continue;

        // We consider only the first element of discreteSuccessors since we are computing it over a single transition.
        const Region &discSucc = discreteSuccessors[0];
        const bool isDiscreteSuccessorInSetG = setG.contains(discSucc);

        // ReSharper disable once CppTooWideScopeInitStatement
        const std::string &actionName = outTrans.action.first;

        // If an action is already present, we update its value only if it is true: if it is false, it must stay false.
        if (validActions.contains(actionName))
        {
            if (validActions[actionName])
                validActions[actionName] = isDiscreteSuccessorInSetG;
        } else
            validActions[actionName] = isDiscreteSuccessorInSetG;
    }

    return validActions;
}


inline void region::RTSArena::everyOutTransitionIsInSetG(const Region &reg,
                                                         const regionSet &setG,
                                                         const std::string &action,
                                                         bool &isRegionValid,
                                                         bool &atLeastOneDiscreteSuccessor) const
{
    for (const auto &outTrans: outTransitions[reg.getLocation()])
    {
        // We only consider transitions with action 'action'.
        if (outTrans.action.first != action)
            continue;

        // ReSharper disable once CppTooWideScopeInitStatement
        const std::vector<Region> discreteSuccessors = reg.getImmediateDiscreteSuccessors({ outTrans }, clocksIndices, locationsToInt);

        // We modify atLeastOneDiscreteSuccessor only if false; if true, it must not be modified again.
        if (!atLeastOneDiscreteSuccessor)
            atLeastOneDiscreteSuccessor = !discreteSuccessors.empty();

        // If at least one discrete successor is not in setG, the region is invalid.
        if (!discreteSuccessors.empty() && !setG.contains(discreteSuccessors[0]))
            isRegionValid = false;
    }
}


inline bool region::RTSArena::piEnvironment(const Region &reg,
                                            const regionSet &setG,
                                            const absl::flat_hash_map<std::string, bool> &validActions,
                                            const bool checkAllSuccessorsInvariants) const
{
    // For every action, we check whether the sequence of delay successors satisfies the condition over the same action.
    for (const auto &[actionName, isValid]: validActions)
    {
        // The action must be valid, otherwise we skip it.
        if (!isValid)
            continue;

        // Needed to ensure that at least one discrete successor is computed, otherwise the game blocks.
        bool atLeastOneDiscreteSuccessor = false;
        bool isRegionValid = true;

        Region oldDelaySucc = reg;
        // ReSharper disable once CppTooWideScopeInitStatement
        Region newDelaySucc = oldDelaySucc.getImmediateDelaySuccessor(maxConstants);

        // Check immediate fixpoint case.
        if (oldDelaySucc == newDelaySucc)
        {
            bool checkOutTransitions = true;

            if (checkAllSuccessorsInvariants)
                checkOutTransitions = !skipRegion(oldDelaySucc, {}, {}, false);

            if (checkOutTransitions)
                everyOutTransitionIsInSetG(oldDelaySucc, setG, actionName, isRegionValid, atLeastOneDiscreteSuccessor);
        } else
        {
            while (oldDelaySucc != newDelaySucc)
            {
                bool checkOutTransitions = true;

                // If the invariants are not satisfied, we simply go on with the next delay successor.
                if (checkAllSuccessorsInvariants)
                    checkOutTransitions = !skipRegion(oldDelaySucc, {}, {}, false);

                if (checkOutTransitions)
                {
                    everyOutTransitionIsInSetG(oldDelaySucc, setG, actionName, isRegionValid, atLeastOneDiscreteSuccessor);

                    // If the region is not valid, by the pi_e condition we can stop checking the sequence of delay successors and try the next action.
                    if (!isRegionValid)
                        break;
                }

                oldDelaySucc = newDelaySucc;
                newDelaySucc = oldDelaySucc.getImmediateDelaySuccessor(maxConstants);
            }

            // The if is needed to skip the computation of everyOutTransitionIsInSetG if isRegionValid is already false.
            if (isRegionValid)
            {
                bool checkOutTransitions = true;

                if (checkAllSuccessorsInvariants)
                    checkOutTransitions = !skipRegion(oldDelaySucc, {}, {}, false);

                if (checkOutTransitions)
                    everyOutTransitionIsInSetG(oldDelaySucc, setG, actionName, isRegionValid, atLeastOneDiscreteSuccessor);
            }
        }

        if (atLeastOneDiscreteSuccessor && isRegionValid)
            return true;
    }

    return false;
}


inline bool region::RTSArena::piController(const absl::flat_hash_map<std::string, bool> &validActions)
{
    // If the controller has at least one valid action from the discrete predecessor computed in piFilter, we can return true.
    return std::ranges::any_of(validActions | std::views::values, std::identity{});
}


inline void region::RTSArena::collectLegalRegionByPi(const Region &reg,
                                                     const regionSet &setG,
                                                     std::vector<Region> &threadLocalRegions,
                                                     const absl::flat_hash_map<std::string, bool> &validActions,
                                                     const bool checkAllSuccessorsInvariants) const
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool isCurrentPlayerController = locationsToPlayers.at(reg.getLocation()) == CONTROLLER;

    if (isCurrentPlayerController ? piController(validActions) : piEnvironment(reg, setG, validActions, checkAllSuccessorsInvariants))
    {
        threadLocalRegions.push_back(reg);

    }
}


inline void region::RTSArena::collectLegalRegionByPiStrategy(const Region &sourceRegion,
                                                             const transition &arenaTransition,
                                                             const Region &targetRegion,
                                                             const clockValuation &cv,
                                                             const regionSet &setG,
                                                             std::vector<Region> &currThreadLocRegions,
                                                             const absl::flat_hash_map<std::string, bool> &validActions,
                                                             const bool checkAllSuccessorsInvariants,
                                                             const bool skipIfSourceIsInSetG)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool isCurrentPlayerController = locationsToPlayers.at(sourceRegion.getLocation()) == CONTROLLER;

    if (isCurrentPlayerController ? piController(validActions) : piEnvironment(sourceRegion, setG, validActions, checkAllSuccessorsInvariants))
    {
        bool collectStrategyTransition = true;

        if (skipIfSourceIsInSetG)
            collectStrategyTransition = !setG.contains(sourceRegion);
#ifdef _OPENMP
        // TODO: It may be beneficial to adopt the same technique for work splitting and merging results as done for threadLocalRegions instead of using critical.
#pragma omp critical
        {
            if (collectStrategyTransition)
                strategyGraph->addStrategyTransition(sourceRegion, arenaTransition, targetRegion, cv);
        }
#else
        if (collectStrategyTransition)
            strategyGraph->addStrategyTransition(sourceRegion, arenaTransition, targetRegion, cv);
#endif
        currThreadLocRegions.push_back(sourceRegion);
    }
}

// Funzione Helper per calcolare e stampare la dimensione reale di un vettore di RegionPtr (come toProcess)
void printToProcessSpecs(const std::vector<RegionPtr>& toProcess) {
    size_t totalBytes = 0;
    for (const auto& ptr : toProcess) {
        if (ptr) {
            // Sfrutta la tua funzione membro per ottenere i byte effettivi dell'oggetto puntato
            totalBytes += ptr->printSizeInBytes(false); 
        }
    }
    
    std::cout << "[TARZAN MPI PROFILING] INPUT 'toProcess' -\n"
              << "  DATO: Numero di elementi: " << toProcess.size() << "\n"
              << "  DATO: Dimensione media elemento: " << (toProcess.empty() ? 0 : totalBytes / toProcess.size()) << " bytes\n"
              << "  DATO: Dimensione totale in memoria: " << totalBytes << " bytes (" 
              << (double)totalBytes / 1024.0 / 1024.0 << " MB)\n" << std::endl;
}

// Funzione Helper per calcolare e stampare la dimensione reale di un regionSet (come setG o intersectionSet)
void printRegionSetSpecs(const std::string& setName, const regionSet& rSet) {
    size_t totalBytes = 0;
    
    // Un unordered_set ha un overhead fisso per i bucket (circa 8 byte a puntatore di bucket)
    size_t bucketOverhead = rSet.bucket_count() * sizeof(void*);
    
    for (const auto& region : rSet) {
        totalBytes += region.printSizeInBytes(false);
        // Ogni nodo di un std::unordered_set introduce un overhead di link (tipicamente 1 o 2 puntatori, ~16 byte)
        totalBytes += 2 * sizeof(void*); 
    }
    
    totalBytes += bucketOverhead;

    std::cout << "[TARZAN MPI PROFILING] INPUT '" << setName << "' -\n"
              << "  DATO: Numero di elementi: " << rSet.size() << "\n"
              << "  DATO: Dimensione media elemento (incluso overhead set): " << (rSet.empty() ? 0 : totalBytes / rSet.size()) << " bytes\n"
              << "  DATO: Dimensione totale in memoria: " << totalBytes << " bytes (" 
              << (double)totalBytes / 1024.0 / 1024.0 << " MB)\n" << std::endl;
}


void region::RTSArena::piFilter(const regionSet &setG,
                                const std::vector<RegionPtr> &toProcess,
                                regionSet &filteredRegions,
                                const regionSet &intersectionSet,
                                bool skipPredecessorsInSetG,
                                bool checkAllSuccessorsInvariants,
                                const bool skipIfSourceIsInSetG)
{

#if DEBUG_MEMORY
    static unsigned long long callCounter = 0;
    callCounter++;
    std::cout << "CHIAMATA piFilter #" << callCounter << "\n";
    
    // printToProcessSpecs(toProcess);
    printRegionSetSpecs("setG", setG);
    // printRegionSetSpecs("intersectionSet", intersectionSet);
#endif

constexpr size_t parallelThreshold = PARALLEL_THRESHOLD;

#ifndef _TBB
    // Thread-local storage for valid predecessors.
#ifdef _OPENMP
    // printf("Working with OMP\n");
    std::vector<std::vector<Region>> threadLocalRegions(omp_get_max_threads());
#else
    std::vector<std::vector<Region>> threadLocalRegions(1);
#endif

    // Reserve space to reduce allocations.
    if (toProcess.size() >= parallelThreshold)
    {
#ifdef _OPENMP
        const size_t estimatedSize = toProcess.size() / omp_get_max_threads();
#else
        const size_t estimatedSize = toProcess.size();
#endif

        for (auto &vec: threadLocalRegions)
            vec.reserve(estimatedSize);
    }

#endif

#ifdef _TBB
    const size_t totalRegions = toProcess.size();
    
    // Usign a threshold here is just degrading the performance
    const bool multi_thread = true;
    const size_t BATCH_SIZE = (multi_thread) ? 2 : totalRegions;
    const size_t numThreads = (multi_thread) ? oneapi::tbb::info::default_concurrency() : 1;
    const size_t estimatedSize = (totalRegions / numThreads) * 2;

    std::vector<std::vector<Region>> threadLocalRegions(numThreads);
    for (auto& vec : threadLocalRegions) {
        vec.reserve(estimatedSize);
    }

    //printf("TBB: Processing %ld regions using a batch of size %ld\n", totalRegions, BATCH_SIZE);

    oneapi::tbb::parallel_for(
        oneapi::tbb::blocked_range<size_t>(0, totalRegions, BATCH_SIZE),
        [&](const oneapi::tbb::blocked_range<size_t>& range) {

        size_t threadIdx = oneapi::tbb::this_task_arena::current_thread_index();

        // Sometimes index can be -1, fallback to slot 0
        if (threadIdx == size_t(-1)) threadIdx = 0;

        std::vector<Region>& currThreadLocRegions = threadLocalRegions[threadIdx];

        for (size_t i = range.begin(); i != range.end(); ++i) {
          const Region& currentRegion = *toProcess[i];
#else
    // The following pragma has default(shared) since apparently there is a bug with OpenMP and std::unordered:set.
    // If a solution is found, put this pragma back to default(none).
#pragma omp parallel for if(toProcess.size() >= parallelThreshold) schedule(dynamic) default(shared) \
shared(setG, toProcess, skipPredecessorsInSetG, intersectionSet, checkAllSuccessorsInvariants, threadLocalRegions), \
shared(inTransitions, outTransitions, clocksIndices, locationsToInt, maxConstants,\
    invariants, initialRegions, locationsToPlayers, computeStrategyGraph, strategyGraph)
    for (int i = 0; i < static_cast<int>(toProcess.size()); i++) // NOLINT(modernize-loop-convert)
    {

        // Getting the current region to process and its incoming transitions.
        const Region &currentRegion = *toProcess[i];

        std::vector<Region>& currThreadLocRegions = 
#ifdef _OPENMP
        threadLocalRegions[omp_get_thread_num()];
#else
        threadLocalRegions[0];
#endif

#endif

        // ReSharper disable once CppTooWideScopeInitStatement
        const std::vector<transition> &currTransitions = inTransitions[currentRegion.getLocation()];

        // We iterate transition by transition for better action handling.
        for (const auto &currTrans: currTransitions)
        {
            // We collect every discrete predecessor that we filter later based on the pi conditions.
            // ReSharper disable once CppTooWideScopeInitStatement
            const std::vector<Region> discPreds = currentRegion.getImmediateDiscretePredecessors({ currTrans }, clocksIndices, locationsToInt, maxConstants);

            // Processing each discrete predecessor to see if a delay predecessor originating from such region can be inserted in filteredRegions.
            for (const auto &reg: discPreds)
            {
                const absl::flat_hash_map<std::string, bool> &validActions = computeValidActions(reg, setG);

                // Since a region may have multiple delay predecessors, we create a deque to process each delay predecessor.
                // Since reg itself must be processed as well, we already add it to delayPredecessorsToProcess.
                std::queue<Region> delayPredecessorsToProcess{};
                delayPredecessorsToProcess.push(reg);

                // Furthermore, we need to later reconsider the regions from which a delay predecessor does not exist.
                regionSet regionsStillToProcess{};

                // We now process each delay predecessor according to the definition of pi as given in our paper.
                while (!delayPredecessorsToProcess.empty())
                {
                    const Region regUnderAnalysis = delayPredecessorsToProcess.front();
                    delayPredecessorsToProcess.pop();

                    // Check whether a region that is the source of a move must be skipped.
                    if (skipRegion(regUnderAnalysis, setG, intersectionSet, skipPredecessorsInSetG))
                        continue;

                    const std::vector<Region> delayPreds = regUnderAnalysis.getImmediateDelayPredecessors();

                    // If a region does not have a delay predecessor, we put 'continue' here, since it will be checked later due to regionsStillToProcess.
                    if (delayPreds.empty())
                    {
                        regionsStillToProcess.insert(regUnderAnalysis);
                        continue;
                    }

                    for (const auto &delayPred: delayPreds)
                        delayPredecessorsToProcess.push(delayPred);

                    // If a delay predecessor has at least one incoming discrete transition (it has at least one discrete predecessor), then it can be the
                    // source of a move, hence we may collect it in filteredRegions.
                    if (regUnderAnalysis.hasAtLeastOneDiscretePredecessor(inTransitions[regUnderAnalysis.getLocation()], clocksIndices))
                    {
                        if (computeStrategyGraph)
                        {
                            collectLegalRegionByPiStrategy(regUnderAnalysis,
                                                           currTrans,
                                                           currentRegion,
                                                           reg.getClockValuation(),
                                                           setG,
                                                           currThreadLocRegions,
                                                           validActions,
                                                           checkAllSuccessorsInvariants,
                                                           skipIfSourceIsInSetG);
                        } else
                            collectLegalRegionByPi(regUnderAnalysis, setG, currThreadLocRegions, validActions, checkAllSuccessorsInvariants);
                    }
                }

                // Checking the case in which a region with no delay predecessors is either initial or has an incoming discrete transition (has a discrete predecessor).
                for (const auto &regStillToProcess: regionsStillToProcess)
                {
                    const bool isRegionInitial = std::ranges::find(initialRegions, regStillToProcess) != initialRegions.end();
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const bool hasDiscPreds = regStillToProcess.hasAtLeastOneDiscretePredecessor(inTransitions[regStillToProcess.getLocation()], clocksIndices);

                    if (isRegionInitial || hasDiscPreds)
                    {
                        if (computeStrategyGraph)
                        {
                            collectLegalRegionByPiStrategy(regStillToProcess,
                                                           currTrans,
                                                           currentRegion,
                                                           reg.getClockValuation(),
                                                           setG,
                                                           currThreadLocRegions,
                                                           validActions,
                                                           checkAllSuccessorsInvariants,
                                                           skipIfSourceIsInSetG);
                        } else
                            collectLegalRegionByPi(regStillToProcess, setG, currThreadLocRegions, validActions, checkAllSuccessorsInvariants);
                    }
                }
            }
        }
    }

#ifdef _TBB
    }, oneapi::tbb::auto_partitioner());
#endif

  mergeResults(threadLocalRegions, filteredRegions);
}


bool region::RTSArena::timedReachability(const regionSet &setPhi, regionSet &setG, std::vector<RegionPtr> &toProcess, const int maxIter)
{
    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start = omp_get_wtime();
#else
    const auto start = std::chrono::high_resolution_clock::now();
#endif

    int currentIteration = 0;
    const int totalStartingRegions = static_cast<int>(setG.size());

    regionSet filteredRegions{};

    if (computeStrategyGraph)
    {
        // Setting the target regions of the strategy graph to match the initial regions.
        strategyGraph->setHeads(initialRegions);

        // Setting the target regions of the strategy graph to match setG.
        strategyGraph->setTargetRegions(setG);
    }

    while (currentIteration < maxIter)
    {
        currentIteration++;

        piFilter(setG, toProcess, filteredRegions, setPhi, true, true, true);

        if (filteredRegions.empty())
            break;

        setG.merge(filteredRegions);

        toProcess.clear();
        toProcess.reserve(setG.size());
        for (const auto &region: setG)
            toProcess.push_back(&region);

        filteredRegions.clear();
    }

    // Ending the timer for measuring computation.
#ifdef _OPENMP
    const auto end = omp_get_wtime();
    const auto duration = end - start;
    std::cout << "Total time:              " << duration * 1000000 << " microseconds" << std::endl;
#else
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Total time:              " << duration << " microseconds" << std::endl;
#endif

    const bool reachable = std::ranges::any_of(initialRegions, [&setG](const auto &region) { return setG.contains(region); });

    std::cout << "Total iterations:        " << currentIteration << std::endl;
    std::cout << "Total starting regions:  " << totalStartingRegions << std::endl;
    std::cout << "Total regions in setPhi: " << setPhi.size() << std::endl;
    std::cout << "Total stored regions:    " << setG.size() << std::endl;
    std::cout << (reachable ? "VICTORY" : "LOSE") << std::endl;

    return reachable;
}


bool region::RTSArena::timedReachability(regionSet &setG, std::vector<RegionPtr> &toProcess, const int maxIter)
{
    return timedReachability({}, setG, toProcess, maxIter);
}


bool region::RTSArena::timedNextReachability(const regionSet &setPhi, regionSet &setG, std::vector<RegionPtr> &toProcess, const int maxIter)
{
    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start = omp_get_wtime();
#else
    const auto start = std::chrono::high_resolution_clock::now();
#endif

    // Step 1: we must compute the fixpoint over (phi UNTIL psi) without checking for an initial region to be reached.
    int currentIteration = 0;
    const int totalStartingRegions = static_cast<int>(setG.size());

    regionSet filteredRegions{};

    if (computeStrategyGraph)
    {
        // Setting the target regions of the strategy graph to match the initial regions.
        strategyGraph->setHeads(initialRegions);

        // Setting the target regions of the strategy graph to match setG.
        strategyGraph->setTargetRegions(setG);
    }

    while (currentIteration < maxIter)
    {
        currentIteration++;

        piFilter(setG, toProcess, filteredRegions, setPhi, true, true, true);

        if (filteredRegions.empty())
            break;

        setG.merge(filteredRegions);

        toProcess.clear();
        toProcess.reserve(setG.size());
        for (const auto &region: setG)
            toProcess.push_back(&region);

        filteredRegions.clear();
    }

    // Step 2: we perform one additional iteration of piFilter.
    toProcess.clear();
    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    filteredRegions.clear();

    // In this case, we remove the constraints over the intersection set setPhi and put skipPredecessorsInSetG to false.
    piFilter(setG, toProcess, filteredRegions, {}, false, true, true);
    setG.merge(filteredRegions);

    // Ending the timer for measuring computation.
#ifdef _OPENMP
    const auto end = omp_get_wtime();
    const auto duration = end - start;
    std::cout << "Total time:              " << duration * 1000000 << " microseconds" << std::endl;
#else
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Total time:              " << duration << " microseconds" << std::endl;
#endif

    const bool reachable = std::ranges::any_of(initialRegions, [&setG](const auto &region) { return setG.contains(region); });

    std::cout << "Total iterations:        " << currentIteration << std::endl;
    std::cout << "Total starting regions:  " << totalStartingRegions << std::endl;
    std::cout << "Total regions in setPhi: " << setPhi.size() << std::endl;
    std::cout << "Total stored regions:    " << setG.size() << std::endl;
    std::cout << (reachable ? "VICTORY" : "LOSE") << std::endl;

    return reachable;
}


bool region::RTSArena::timedSafety(regionSet &setG, std::vector<RegionPtr> &toProcess, const int maxIter)
{
    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start = omp_get_wtime();
#else
    const auto start = std::chrono::high_resolution_clock::now();
#endif

    int currentIteration = 0;
    const int totalStartingRegions = static_cast<int>(setG.size());

    regionSet filteredRegions{};

    // Used to enable or disable the strategy transition computation.
    const bool reactivateComputeStrategyGraph = computeStrategyGraph;

    if (computeStrategyGraph)
    {
        // Setting the target regions of the strategy graph to match the initial regions.
        // Here we do not need to set the targets, as they will be updated in the safety strategy synthesis algorithm.
        strategyGraph->setHeads(initialRegions);

        computeStrategyGraph = false;
    }

    while (currentIteration < maxIter)
    {
        currentIteration++;

        const size_t oldSetGSize = setG.size();

        piFilter(setG, toProcess, filteredRegions, {}, false, true, false);

        // Computing the intersection between regions returned by piFilter and setG.
        std::erase_if(setG, [&filteredRegions](const auto &region) { return !filteredRegions.contains(region); });

        if (oldSetGSize == setG.size())
            break;

        toProcess.clear();
        toProcess.reserve(setG.size());
        for (const auto &region: setG)
            toProcess.push_back(&region);

        filteredRegions.clear();
    }

    computeStrategyGraph = reactivateComputeStrategyGraph;

    // If the strategy graph must be computed, here we perform one last piFilter application to compute the strategy transitions.
    if (computeStrategyGraph)
        piFilter(setG, toProcess, filteredRegions, {}, false, true, false);

    // Ending the timer for measuring computation.
#ifdef _OPENMP
    const auto end = omp_get_wtime();
    const auto duration = end - start;
    std::cout << "Total time:             " << duration * 1000000 << " microseconds" << std::endl;
#else
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Total time:             " << duration << " microseconds" << std::endl;
#endif

    const bool reachable = std::ranges::any_of(initialRegions, [&setG](const auto &region) { return setG.contains(region); });

    std::cout << "Total iterations:       " << currentIteration << std::endl;
    std::cout << "Total starting regions: " << totalStartingRegions << std::endl;
    std::cout << "Total stored regions:   " << setG.size() << std::endl;
    std::cout << (reachable ? "VICTORY" : "LOSE") << std::endl;

    return reachable;
}


inline bool region::RTSArena::solveGameWithBoxFormula(const cltloc::ast::unaryCLTLocFormula &unaryFormula)
{
    std::vector<regionSet> startingRegions = getRegionsFromGeneralCLTLocFormula(unaryFormula.rightFormula);

    if (startingRegions.size() != 1)
        throw std::logic_error("Wrong size of unary formula.");

    regionSet &setG = startingRegions[0];

    std::vector<RegionPtr> toProcess{};
    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    return timedSafety(setG, toProcess, MAX_ITERATIONS);
}


inline bool region::RTSArena::solveGameWithDiamondFormula(const cltloc::ast::unaryCLTLocFormula &unaryFormula)
{
    std::vector<regionSet> startingRegions = getRegionsFromGeneralCLTLocFormula(unaryFormula.rightFormula);

    if (startingRegions.size() != 1)
        throw std::logic_error("Wrong size of unary formula.");

    regionSet &setG = startingRegions[0];

    std::vector<RegionPtr> toProcess{};
    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    return timedReachability(setG, toProcess, MAX_ITERATIONS);
}


inline bool region::RTSArena::solveGameWithUntilFormula(const cltloc::ast::binaryCLTLocFormula &binaryFormula)
{
    const std::vector<regionSet> leftRegions = getRegionsFromGeneralCLTLocFormula(binaryFormula.leftFormula);
    std::vector<regionSet> rightRegions = getRegionsFromGeneralCLTLocFormula(binaryFormula.rightFormula);

    if (leftRegions.size() != 1 || rightRegions.size() != 1)
        throw std::logic_error("Wrong size of binary formula.");

    const regionSet &setPhi = leftRegions[0];
    regionSet &setG = rightRegions[0];

    std::vector<RegionPtr> toProcess{};
    toProcess.reserve(setG.size());
    for (const auto &region: setG)
        toProcess.push_back(&region);

    return timedReachability(setPhi, setG, toProcess, MAX_ITERATIONS);
}


inline bool region::RTSArena::solveGameWithNextFormula(const cltloc::ast::generalCLTLocFormula &formula)
{
    // The formula must be of the form: phi UNTIL psi.
    const bool result = boost::apply_visitor([this]<typename T0>(T0 const &val) -> bool {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::binaryCLTLocFormula>>)
        {
            const auto &binaryFormula = val.get();

            const std::vector<regionSet> leftRegions = getRegionsFromGeneralCLTLocFormula(binaryFormula.leftFormula);
            std::vector<regionSet> rightRegions = getRegionsFromGeneralCLTLocFormula(binaryFormula.rightFormula);

            if (leftRegions.size() != 1 || rightRegions.size() != 1)
                throw std::logic_error("Wrong size of binary formula.");

            const regionSet &setPhi = leftRegions[0];
            regionSet &setG = rightRegions[0];

            std::vector<RegionPtr> toProcess{};
            toProcess.reserve(setG.size());
            for (const auto &region: setG)
                toProcess.push_back(&region);

            return timedNextReachability(setPhi, setG, toProcess, MAX_ITERATIONS);
        } else
            throw std::logic_error("When solving a NEXT formula it must be of the form: NEXT (phi UNTIL psi).");
    }, formula.value);

    return result;
}


bool region::RTSArena::solveTimedCLTLocGame(const cltloc::ast::generalCLTLocFormula &formula)
{
    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start = omp_get_wtime();
#else
    const auto start = std::chrono::high_resolution_clock::now();
#endif

    const bool result = boost::apply_visitor([this]<typename T0>(T0 const &val) -> bool {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::pureCLTLocFormula>>)
        {
            // Pure formula: currently unhandled.
            throw std::logic_error("Pure formulae are not currently supported when solving Timed CLTLoc Games.");
            // ---
        } else if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::unaryCLTLocFormula>>)
        {
            // Unary formula.
            switch (const auto &unaryFormula = val.get(); unaryFormula.op)
            {
                case BOX:
                    return solveGameWithBoxFormula(unaryFormula);

                case DIAMOND:
                    return solveGameWithDiamondFormula(unaryFormula);

                case NEXT:
                    return solveGameWithNextFormula(unaryFormula.rightFormula);

                default:
                    throw std::logic_error("Invalid unary CLTLoc operator.");
            }
            // ---
        } else if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::binaryCLTLocFormula>>)
        {
            // Binary formula.
            switch (const auto &binaryFormula = val.get(); binaryFormula.op)
            {
                case UNTIL:
                    return solveGameWithUntilFormula(binaryFormula);

                default:
                    throw std::logic_error("Invalid binary CLTLoc operator.");
            }
            // ---
        } else
            throw std::logic_error("Unhandled formula type in solveTimedCLTLocGame.");
    }, formula.value);

    // Ending the timer for measuring computation.
#ifdef _OPENMP
    const auto end = omp_get_wtime();
    const auto duration = end - start;
    std::cout << "Total time including region generation: " << duration * 1000000 << " microseconds" << std::endl;
#else
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Total time including region generation: " << duration << " microseconds" << std::endl;
#endif

    return result;
}

inline bool region::RTSArena::solveGameWithNestedUntilConjunction(const std::vector<cltloc::ast::generalCLTLocFormula> &formulae)
{

  if (formulae.empty())
    throw std::logic_error("Formulae vector is empty!");

  const int formulaeSize = static_cast<int>(formulae.size());

  // Storage for valid region sets. At position i, the vector will contain the regions specified by formulae[i].
#ifdef _OPENMP
  std::vector<regionSet> formulaRegionSets(formulaeSize);
#else
  std::vector<regionSet> formulaRegionSets{};
#endif

#pragma omp parallel for schedule(dynamic) default(none) shared(formulaRegionSets, formulaeSize, formulae)
  for (int i = 0; i < formulaeSize; i++)
  {
    const std::vector<regionSet> formulaRegions = getRegionsFromGeneralCLTLocFormula(formulae[i]);

    if (formulaRegions.size() != 1)
      throw std::logic_error("Wrong size of unary formula!");

#ifdef _OPENMP
    formulaRegionSets[i] = formulaRegions.at(0);
#else
    formulaRegionSets.push_back(formulaRegions.at(0));
#endif
  }

  // Defining the starting set of states used during computation (it corresponds to the set in the back of formulaRegionSets).
  regionSet setG = std::move(formulaRegionSets.back());
  std::vector<RegionPtr> toProcess{};

  for (const auto &region: setG)
    toProcess.push_back(&region);

  int currentIteration = 0;
  const int totalStartingRegions = static_cast<int>(setG.size());

  for (int i = formulaeSize - 1; i > 0; i--) 
  {
    regionSet currentStepSet = setG; 
    bool changed = true;

    while (changed) {
      size_t sizeBefore = currentStepSet.size();

      regionSet predecessors{};
      piFilter(currentStepSet, toProcess, predecessors, {}, false, false, false);

      const regionSet &targetFormula = formulaRegionSets[i - 1];
      std::erase_if(predecessors, [&targetFormula](const auto &reg) { 
          return !targetFormula.contains(reg); 
          });

      currentStepSet.merge(predecessors);

      if (currentStepSet.size() == sizeBefore) changed = false;

      toProcess.clear();
      toProcess.reserve(setG.size());

      for (const auto &region : currentStepSet) toProcess.push_back(&region);
    }

    setG = std::move(currentStepSet);
    currentIteration++;
  }

  const bool reachable = std::ranges::any_of(initialRegions, [&setG](const auto &region) { return setG.contains(region); });

  std::cout << "Total iterations:       " << currentIteration << std::endl;
  std::cout << "Total starting regions: " << totalStartingRegions << std::endl;
  std::cout << "Total stored regions:   " << setG.size() << std::endl;
  std::cout << (reachable ? "VICTORY" : "LOSE") << std::endl;

  return reachable;
}

inline bool region::RTSArena::solveGameWithAndNextConjunction(const std::vector<cltloc::ast::generalCLTLocFormula> &formulae)
{
    if (formulae.empty())
        throw std::logic_error("Formulae vector is empty!");

    const int formulaeSize = static_cast<int>(formulae.size());

    // Storage for valid region sets. At position i, the vector will contain the regions specified by formulae[i].
#ifdef _OPENMP
    std::vector<regionSet> formulaRegionSets(formulaeSize);
#else
    std::vector<regionSet> formulaRegionSets{};
#endif

#pragma omp parallel for schedule(dynamic) default(none) shared(formulaRegionSets, formulaeSize, formulae)
    for (int i = 0; i < formulaeSize; i++)
    {
        const std::vector<regionSet> formulaRegions = getRegionsFromGeneralCLTLocFormula(formulae[i]);

        if (formulaRegions.size() != 1)
            throw std::logic_error("Wrong size of unary formula!");

#ifdef _OPENMP
        formulaRegionSets[i] = formulaRegions.at(0);
#else
        formulaRegionSets.push_back(formulaRegions.at(0));
#endif
    }

    // Defining the starting set of states used during computation (it corresponds to the set in the back of formulaRegionSets).
    regionSet setG = std::move(formulaRegionSets.back());
    std::vector<RegionPtr> toProcess{};
    regionSet filteredRegions{};

    for (const auto &region: setG)
        toProcess.push_back(&region);

    int currentIteration = 0;
    const int totalStartingRegions = static_cast<int>(setG.size());

    if (computeStrategyGraph)
    {
        // Setting the heads of the strategy graph to match the initial regions.
        strategyGraph->setHeads(initialRegions);

        // Setting the target regions of the strategy graph to match the back of formulaRegions, that is, the initial content of setG.
        strategyGraph->setTargetRegions(setG);
    }

    // The computation proceeds backwards from the last (i.e., from the back) formula in the formulaRegionSets vector.
    // Depending on its applicationCount, we apply piFilter until another formula is met (going backwards).
    // When this happens, we intersect its states with the ones collected up to the current iteration.
    for (int i = formulaeSize - 1; i > 0; i--)
    {
        // Getting the current applicationCount value.
        const auto currCount = boost::get<boost::spirit::x3::forward_ast<cltloc::ast::unaryCLTLocFormula>>(formulae[i].value).get().applicationCount;

        // Getting the applicationCount value of the formula that is met going backwards.
        const auto backCount = boost::get<boost::spirit::x3::forward_ast<cltloc::ast::unaryCLTLocFormula>>(formulae[i - 1].value).get().applicationCount;

        if (!currCount.has_value() || !backCount.has_value())
            throw std::logic_error("No currCount or backCount value!");

        if (currCount.value() <= backCount.value())
            throw std::logic_error("Invalid applicationCount values: currCount <= backCount!");

        const int totalCurrentIterations = currCount.value() - backCount.value();

        // We now apply piFilter for a total of totalCurrentIterations times.
        for (int j = 0; j < totalCurrentIterations; j++)
        {
            currentIteration++;

            // Each step of the conjunction will be saved backwards (at the end of computation, the back of strategyTransitionsForConjunction contains the
            // first strategy transition and so on). For this reason, in this algorithm we add maps to strategyTransitionsForConjunction at each step of the game.
            if (computeStrategyGraph)
                strategyGraph->addNewStrategyTransitionMapToBack();

            // Here we put skipPredecessorsInSetG and skipIfSourceIsInSetG to false, since we may need to traverse cycles in the conjunction of next.
            piFilter(setG, toProcess, filteredRegions, {}, false, true, false);

            setG.merge(filteredRegions);
            filteredRegions.clear();

            // We can skip this during the last iteration since toProcess will be computed again after the intersection (see below).
            if (j < totalCurrentIterations - 1)
            {
                toProcess.clear();
                toProcess.reserve(setG.size());
                for (const auto &region: setG)
                    toProcess.push_back(&region);
            }
        }

        // Computing the intersection between setG and the regions specified by formulaRegionSets[i - 1].
        const regionSet &target = formulaRegionSets[i - 1];
        std::erase_if(setG, [&target](const auto &region) { return !target.contains(region); });

        // Since we are restricting setG, we must restrict the collected strategy transitions as well.
        if (computeStrategyGraph)
            strategyGraph->eraseUnnecessaryTransitions(setG);

        toProcess.clear();
        toProcess.reserve(setG.size());
        for (const auto &region: setG)
            toProcess.push_back(&region);
    }

    // Now only the last formula (the one in the first position of formulaRegionSets) must be checked.
    const auto count = boost::get<boost::spirit::x3::forward_ast<cltloc::ast::unaryCLTLocFormula>>(formulae[0].value).get().applicationCount;

    if (!count.has_value())
        throw std::logic_error("No applicationCount value!");

    const int totalCurrentIterations = count.value();

    for (int i = 0; i < totalCurrentIterations; i++)
    {
        currentIteration++;

        // Each step of the conjunction will be saved backwards (at the end of computation, the back of strategyTransitionsForConjunction contains the
        // first strategy transition and so on). For this reason, in this algorithm we add maps to strategyTransitionsForConjunction at each step of the game.
        if (computeStrategyGraph)
            strategyGraph->addNewStrategyTransitionMapToBack();

        // Here we put skipPredecessorsInSetG and skipIfSourceIsInSetG to false, since we may need to traverse cycles in the conjunction of next.
        piFilter(setG, toProcess, filteredRegions, {}, false, true, false);

        setG.merge(filteredRegions);
        filteredRegions.clear();

        // We can skip this during the last iteration.
        if (i < totalCurrentIterations - 1)
        {
            toProcess.clear();
            toProcess.reserve(setG.size());
            for (const auto &region: setG)
                toProcess.push_back(&region);
        }
    }

    const bool reachable = std::ranges::any_of(initialRegions, [&setG](const auto &region) { return setG.contains(region); });

    std::cout << "Total iterations:       " << currentIteration << std::endl;
    std::cout << "Total starting regions: " << totalStartingRegions << std::endl;
    std::cout << "Total stored regions:   " << setG.size() << std::endl;
    std::cout << (reachable ? "VICTORY" : "LOSE") << std::endl;

    return reachable;
}


bool region::RTSArena::solveTimedCLTLocGame(const cltloc::ast::conjunctionOfFormulae &conjunction)
{
    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start = omp_get_wtime();
#else
    const auto start = std::chrono::high_resolution_clock::now();
#endif

    bool result{};

    switch (conjunction.type)
    {
        case AND_GENERAL:
            std::cout << "Support for general conjunction still have to be implemented." << std::endl;
            throw std::logic_error("Invalid conjunction type.");

        case AND_NEXT:
            result = solveGameWithAndNextConjunction(conjunction.formulae);
            break;

        case NESTED_UNTIL:
            result = solveGameWithNestedUntilConjunction(conjunction.formulae);
            break;

        default:
            throw std::logic_error("Invalid conjunction type.");
    }

    // Ending the timer for measuring computation.
#ifdef _OPENMP
    const auto end = omp_get_wtime();
    const auto duration = end - start;
    std::cout << "Total time including region generation: " << duration * 1000000 << " microseconds" << std::endl;
#else
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Total time including region generation: " << duration << " microseconds" << std::endl;
#endif

    return result;
}


void region::RTSArena::printReachabilityPlay(const std::unordered_map<int, std::string> &indicesToClocks) const
{
    const auto &targetRegions = strategyGraph->getTargetRegions();

    std::cout << "\n";
    std::cout << "  \u2554" << repeatString("\u2550", BOX_WIDTH) << "\u2557\n";
    std::cout << "  \u2551   WINNING CONTROLLER REACHABILITY PLAY   \u2551\n";
    std::cout << "  \u255a" << repeatString("\u2550", BOX_WIDTH) << "\u255d\n\n";

    // We assume to always take the first region in heads as the starting one.
    Region current = strategyGraph->getHeads()[0];
    int step = 0;

    while (!targetRegions.contains(current))
    {
        const strategyTransitionSet strategyTransSet = strategyGraph->getStrategyTransitionsGivenSource(current);

        if (strategyTransSet.empty())
            throw std::logic_error("No outgoing strategy transitions from current region!");

        // We assume to always take the first available transition in the strategy transition set.
        // Since the reachability strategy graph is cycle-free, every transition guarantees progress for the controller to a target region.
        const auto &[arenaTransition, target, moveClockValuation] = *strategyTransSet.begin();

        StrategyGraph::printStrategyTransition(step, current, intToLocations, indicesToClocks, locationsToPlayers, moveClockValuation, arenaTransition);

        current = target;
        step++;
    }

    // Print the final region (the target).
    StrategyGraph::printRegionWithBox(step, current, intToLocations, indicesToClocks, locationsToPlayers, " \u2605");
}


void region::RTSArena::printSafetyPlay(const std::unordered_map<int, std::string> &indicesToClocks) const
{
    auto targetRegions = strategyGraph->getTargetRegions();

    std::cout << "\n";
    std::cout << "  \u2554" << repeatString("\u2550", BOX_WIDTH) << "\u2557\n";
    std::cout << "  \u2551      WINNING CONTROLLER SAFETY PLAY      \u2551\n";
    std::cout << "  \u255a" << repeatString("\u2550", BOX_WIDTH) << "\u255d\n\n";

    // We assume to always take the first region in heads as the starting one.
    Region current = strategyGraph->getHeads()[0];
    int step = 0;

    // Emplacing the current region (corresponding to an initial region) to also detect cycles involving initial regions.
    targetRegions.emplace(current);

    // This loop will continue to execute until a cycle in the strategy graph is found.
    while (step <= MAX_ITERATIONS)
    {
        const strategyTransitionSet strategyTransSet = strategyGraph->getStrategyTransitionsGivenSource(current);

        if (strategyTransSet.empty())
            throw std::logic_error("No outgoing strategy transitions from current region!");

        // We assume to always take the first available transition in the strategy transition set.
        // Since every transition guarantees the controller stays in the safety set, it will eventually encounter a cycle.
        const auto &[arenaTransition, target, moveClockValuation] = *strategyTransSet.begin();

        StrategyGraph::printStrategyTransition(step, current, intToLocations, indicesToClocks, locationsToPlayers, moveClockValuation, arenaTransition);

        step++;
        current = target;

        if (targetRegions.contains(target))
            break;

        targetRegions.emplace(target);
    }

    // Print the region from which the cycle starts.
    StrategyGraph::printRegionWithBox(step, current, intToLocations, indicesToClocks, locationsToPlayers, " \u2605");
}


void region::RTSArena::printPlay(const cltloc::ast::generalCLTLocFormula &formula)
{
    if (!computeStrategyGraph)
        throw CannotSynthesizeStrategiesException("The parameter 'computeStrategyGraph' is set to false!");

    if (!solveTimedCLTLocGame(formula))
        throw CannotSynthesizeStrategiesException("A controller winning strategy does not exist!");

    const auto &indicesToClocks = getIndicesToClocksMap();

    // We now synthesize a winning controller play based on the winning condition type.
    boost::apply_visitor([this, indicesToClocks]<typename T0>(T0 const &val)
    {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::pureCLTLocFormula>>)
        {
            // Pure formula: currently unhandled.
            throw std::logic_error("Pure formulae are not currently supported when solving Timed CLTLoc Games.");
            // ---
        } else if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::unaryCLTLocFormula>>)
        {
            // Unary formula.
            // TODO: la sintesi del next unario per ora funziona perchè si assume che l'unico next unario sia next until.
            switch (const auto &unaryFormula = val.get(); unaryFormula.op)
            {
                case BOX:
                    printSafetyPlay(indicesToClocks);
                    break;

                case DIAMOND:
                case NEXT:
                    printReachabilityPlay(indicesToClocks);
                    break;

                default:
                    throw std::logic_error("Invalid CLTLoc formula for synthesis.");
            }
            // ---
        } else if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::binaryCLTLocFormula>>)
        {
            // Binary formula.
            switch (const auto &binaryFormula = val.get(); binaryFormula.op)
            {
                case UNTIL:
                    printReachabilityPlay(indicesToClocks);
                    break;

                default:
                    throw std::logic_error("Invalid CLTLoc formula for synthesis.");
            }
            // ---
        } else
            throw std::logic_error("Invalid CLTLoc formula for synthesis.");
    }, formula.value);
}


void region::RTSArena::printAndNextConjunctionPlay(const std::unordered_map<int, std::string> &indicesToClocks) const
{
    std::cout << "\n";
    std::cout << "  \u2554" << repeatString("\u2550", BOX_WIDTH) << "\u2557\n";
    std::cout << "  \u2551     WINNING CONTROLLER AND_NEXT PLAY     \u2551\n";
    std::cout << "  \u255a" << repeatString("\u2550", BOX_WIDTH) << "\u255d\n\n";

    // We assume to always take the first region in heads as the starting one.
    Region current = strategyGraph->getHeads()[0];

    // Dynamic cast to get the strategy transition vector for the conjunction.
    const auto *sg_p = dynamic_cast<StrategyGraphForConjunction *>(strategyGraph.get());
    if (!sg_p)
        throw std::logic_error("Expected a StrategyGraphForConjunction!");
    const auto &strategyTransitionsForConjunction = sg_p->getStrategyTransitionsForConjunction();

    int step{};
    const int mStep = static_cast<int>(strategyTransitionsForConjunction.size());

    for (step = mStep - 1; step >= 0; step--)
    {
        const strategyTransitionSet strategyTransSet = strategyGraph->getStrategyTransitionsGivenSourceAndIndex(current, step);

        if (strategyTransSet.empty())
            throw std::logic_error("No outgoing strategy transitions from current region!");

        // We assume to always take the first available transition in the strategy transition set.
        const auto &[arenaTransition, target, moveClockValuation] = *strategyTransSet.begin();

        const int fixedStep = mStep - step - 1;
        StrategyGraph::printStrategyTransition(fixedStep, current, intToLocations, indicesToClocks, locationsToPlayers, moveClockValuation, arenaTransition);

        current = target;
    }

    // Print the final region (the target).
    StrategyGraph::printRegionWithBox(mStep - step - 1, current, intToLocations, indicesToClocks, locationsToPlayers, " \u2605");
}


void region::RTSArena::printPlay(const cltloc::ast::conjunctionOfFormulae &conjunction)
{
    if (!computeStrategyGraph)
        throw CannotSynthesizeStrategiesException("The parameter 'computeStrategyGraph' is set to false!");

    if (!solveTimedCLTLocGame(conjunction))
        throw CannotSynthesizeStrategiesException("A controller winning strategy does not exist!");

    const auto &indicesToClocks = getIndicesToClocksMap();

    // We now synthesize a winning controller strategy based on the winning conjunction type.
    switch (conjunction.type)
    {
        case AND_NEXT:
            printAndNextConjunctionPlay(indicesToClocks);
            break;

        default:
            throw std::logic_error("Invalid conjunction type.");
    }
}


void region::RTSArena::strategyGraphToDot(const std::string &path, const cltloc::ast::generalCLTLocFormula &formula)
{
    if (!computeStrategyGraph)
        throw CannotSynthesizeStrategiesException("The parameter 'computeStrategyGraph' is set to false!");

    const auto &indicesToClocks = getIndicesToClocksMap();

    if (solveTimedCLTLocGame(formula))
        return strategyGraph->to_dot(path, indicesToClocks, intToLocations, locationsToPlayers);

    throw std::runtime_error("No winning controller strategy exists, hence the strategy graph cannot be computed!");
}


void region::RTSArena::strategyGraphToDot(const std::string &path, const cltloc::ast::conjunctionOfFormulae &conjunction)
{
    if (!computeStrategyGraph)
        throw CannotSynthesizeStrategiesException("The parameter 'computeStrategyGraph' is set to false!");

    const auto &indicesToClocks = getIndicesToClocksMap();

    if (solveTimedCLTLocGame(conjunction))
        return strategyGraph->to_dot(path, indicesToClocks, intToLocations, locationsToPlayers);

    throw std::runtime_error("No winning controller strategy exists, hence the strategy graph cannot be computed!");
}


std::string region::RTSArena::to_string() const
{
    std::ostringstream oss;

    oss << "=== RTS Information ===\n\n";

    // Clock indices.
    oss << "Clocks (" << clocksIndices.size() << "):\n";
    for (const auto &[clockName, index]: clocksIndices)
        oss << "  " << clockName << " -> index " << index << "\n";
    oss << "\n";

    // Locations.
    oss << "Locations (" << locationsToInt.size() << "):\n";
    for (const auto &[locationName, index]: locationsToInt)
        oss << "  " << locationName << " -> " << index << "\n";
    oss << "\n";

    // Max constants.
    oss << "Max Constants (" << maxConstants.size() << "):\n";
    for (size_t i = 0; i < maxConstants.size(); ++i)
        oss << "  Clock " << i << ": " << maxConstants[i] << "\n";
    oss << "\n";

    // Initial locations.
    oss << "Initial Locations (" << initialLocations.size() << "):\n";
    for (const int loc: initialLocations)
        oss << "  " << loc << "\n";
    oss << "\n";

    // Outgoing transitions.
    oss << "Outgoing Transitions (" << outTransitions.size() << " locations):\n";
    for (size_t loc = 0; loc < outTransitions.size(); ++loc)
    {
        if (!outTransitions[loc].empty())
        {
            oss << "  From location " << loc << " (" << outTransitions[loc].size() << " transitions):\n";
            for (const auto &trans: outTransitions[loc])
                oss << "    " << trans.startingLocation << " -> " << trans.targetLocation << "\n";
        }
    }
    oss << "\n";

    // Incoming transitions.
    oss << "Incoming Transitions (" << inTransitions.size() << " locations):\n";
    for (size_t loc = 0; loc < inTransitions.size(); ++loc)
    {
        if (!inTransitions[loc].empty())
        {
            oss << "  To location " << loc << " (" << inTransitions[loc].size() << " transitions):\n";
            for (const auto &trans: inTransitions[loc])
                oss << "    " << trans.startingLocation << " -> " << trans.targetLocation << "\n";
        }
    }
    oss << "\n";

    // Initial regions.
    oss << "Initial Regions (" << initialRegions.size() << "):\n";
    for (size_t i = 0; i < initialRegions.size(); ++i)
        oss << "  Region " << i << ":\n" << initialRegions[i].toString() << "\n";

    // Invariants.
    oss << "Invariants (" << invariants.size() << " locations with invariants):\n";
    for (const auto &[loc, constraints]: invariants)
        oss << "  Location " << loc << " (" << constraints.size() << " constraints)\n";
    oss << "\n";

    // Urgent locations.
    oss << "Urgent Locations (" << urgentLocations.size() << "):\n";
    if (urgentLocations.empty())
        oss << "  (none)\n";
    else
        for (const int loc: urgentLocations)
            oss << "  " << loc << "\n";
    oss << "\n";

    // Locations to players (arena-specific, may be empty).
    oss << "Locations to Players (" << locationsToPlayers.size() << "):\n";
    if (locationsToPlayers.empty())
        oss << "  (none)\n";
    else
        for (const auto &[loc, player]: locationsToPlayers)
            oss << "  Location " << loc << " -> Player '" << player << "'\n";

    return oss.str();
}
