#include "RTSArena.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/exceptions/nestedCLTLocFormula_exception.h"
#include "TARZAN/utilities/function_utilities.h"

#ifdef _OPENMP
#include <omp.h>
#endif


// #define RTSARENA_DEBUG
#define THROW_NESTEDCLTLOC_EXCEPTION

// Threshold for enabling parallel execution (derived from manual experiments).
#define PARALLEL_THRESHOLD 400

// Total number of iterations performed by fixed point algorithms.
#define MAX_ITERATIONS 10000


std::unordered_set<region::Region, region::RegionHash> region::RTSArena::getRegionsFromPureCLTLocFormula(const cltloc::ast::pureCLTLocFormula &formula) const
{
#ifdef RTSARENA_DEBUG

    std::cout << "Now handling the following pure CLTLoc formula: " << formula << std::endl;

#endif

    const int nClocks = static_cast<int>(clocksIndices.size());

    regionSet res = Region::generateRegionsFromConstraints(formula.locations, formula.clockConstraints, clocksIndices, locationsToInt, maxConstants, nClocks);

    // Removing regions that do not satisfy the invariants of the Timed Arena.
    std::erase_if(res, [this](const Region &reg) {
        if (const auto it = invariants.find(reg.getLocation()); it != invariants.end())
            return !isInvariantSatisfied(it->second, reg.getClockValuation(), clocksIndices);
        return false; // No invariant = keep the region.
    });

    return res;
}


// TODO: per ora l'implementazione corrente va bene perchè non hai annidamenti e quindi hai al più due vettori nel vettore esterno risultante.
//       Vedere di trovare il modo di rendere il tutto più generale qualora vengano resi disponibili livelli di annidamento nelle formule.
inline std::vector<regionSet> region::RTSArena::getRegionsFromGeneralCLTLocFormulaWithDepth(const cltloc::ast::generalCLTLocFormula &formula,
                                                                                            int depth) const // NOLINT
{
    std::vector<regionSet> res{};

    std::visit([this, &res, depth]<typename T0>(T0 const &val) {
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
    if (const auto it = invariants.find(reg.getLocation()); it != invariants.end())
        if (!isInvariantSatisfied(it->second, reg.getClockValuation(), clocksIndices))
            return true;

    return false;
}


inline void region::RTSArena::mergeResults(const std::vector<std::vector<Region>> &threadLocalRegions, regionSet &filteredRegions)
{
    // Sequential merge of thread-local results.
    for (const auto &localRegions: threadLocalRegions)
        for (const auto &reg: localRegions)
            filteredRegions.insert(reg);
}


inline bool region::RTSArena::delaySuccessorsCheck(const Region &reg, const regionSet &setG, const bool checkAllSuccessorsInvariants) const
{
    bool isRegionValid = true;

    Region oldDelaySucc = reg;
    // ReSharper disable once CppTooWideScopeInitStatement
    Region newDelaySucc = oldDelaySucc.getImmediateDelaySuccessor(maxConstants);

    // Check immediate fixpoint case, only valid if reg is in setG.
    if (oldDelaySucc == newDelaySucc)
        isRegionValid = setG.contains(reg);
    else
    {
        while (oldDelaySucc != newDelaySucc)
        {
            // The environment cannot bring the game into a deadlock state by waiting an arbitrary amount of time.
            // Boolean to track whether all outgoing transitions of an immediate delay successor are disabled.
            // Here we want at least one such transition to be enabled for reg to be valid.
            bool allDisabled = true;
            // ReSharper disable once CppTooWideScopeInitStatement
            const std::vector<transition> &newDelaySuccTransitions = outTransitions[newDelaySucc.getLocation()];
            for (const auto &transition: newDelaySuccTransitions)
                if (transition.isTransitionSatisfied(newDelaySucc.getClockValuation(), clocksIndices, {}))
                {
                    allDisabled = false;
                    break;
                }

            // If all transitions are disabled, simply continue by checking the next immediate delay successor.
            if (allDisabled)
            {
                oldDelaySucc = newDelaySucc;
                newDelaySucc = oldDelaySucc.getImmediateDelaySuccessor(maxConstants);
            } else
            {
                // If the delay successor is not in setG, the region is invalid.
                if (!setG.contains(newDelaySucc))
                {
                    isRegionValid = false;
                    break;
                }

                // In safety formulae, it may be necessary to satisfy the invariants for immediate delay successors.
                if (checkAllSuccessorsInvariants)
                    if (const auto it = invariants.find(newDelaySucc.getLocation()); it != invariants.end())
                        if (!isInvariantSatisfied(it->second, newDelaySucc.getClockValuation(), clocksIndices))
                        {
                            isRegionValid = false;
                            break;
                        }

                oldDelaySucc = newDelaySucc;
                newDelaySucc = oldDelaySucc.getImmediateDelaySuccessor(maxConstants);
            }
        }
    }

    return isRegionValid;
}


void region::RTSArena::omegaFilter(const regionSet &setG,
                                   const std::vector<RegionPtr> &toProcess,
                                   regionSet &filteredRegions,
                                   const regionSet &intersectionSet,
                                   const bool skipPredecessorsInSetG,
                                   const bool checkAllSuccessorsInvariants) const
{
    constexpr size_t parallelThreshold = PARALLEL_THRESHOLD;

    // Thread-local storage for valid predecessors.
#ifdef _OPENMP
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

#pragma omp parallel for if(toProcess.size() >= parallelThreshold) schedule(dynamic) default(none) \
shared(setG, toProcess, skipPredecessorsInSetG, intersectionSet, checkAllSuccessorsInvariants, threadLocalRegions), \
shared(inTransitions, outTransitions, clocksIndices, locationsToInt, maxConstants, invariants)
    for (int i = 0; i < static_cast<int>(toProcess.size()); i++) // NOLINT(modernize-loop-convert)
    {
        // Getting the current region to process and its incoming transitions.
        const Region &currentRegion = *toProcess[i];
        const std::vector<transition> &currTransitions = inTransitions[currentRegion.getLocation()];

        // We collect every discrete predecessor that we filter later based on the omega filter requirements.
        // ReSharper disable once CppTooWideScopeInitStatement
        const std::vector<Region> discPreds = currentRegion.getImmediateDiscretePredecessors(currTransitions, clocksIndices, locationsToInt, maxConstants);

        // Processing each discrete predecessor to see if it can be inserted in filteredRegions.
        for (const auto &reg: discPreds)
        {
            if (skipRegion(reg, setG, intersectionSet, skipPredecessorsInSetG))
                continue;

            bool isRegionValid{};
            const std::vector<transition> &regOutTransitions = outTransitions[reg.getLocation()];

            // Building a map from actions names to transitions indices to ease the check required by the omega filter.
            absl::flat_hash_map<std::string, std::vector<int>> actionsToTransitionIndices{};
            for (int tIdx = 0; tIdx < static_cast<int>(regOutTransitions.size()); tIdx++)
                actionsToTransitionIndices[regOutTransitions[tIdx].action.first].push_back(tIdx);

            // Track which actions have been processed to avoid redundant checks.
            absl::flat_hash_set<std::string> processedActions{};

            // Outgoing transitions must be such that (at least one transition must satisfy these requirements for a region to be valid):
            // - If its action is unique, it must lead to a region in setG, or
            // - If its action is not unique, all other transitions with the same action must lead to a region in setG.
            // TODO: we can stop at the first action satisfying the above requirements, since we are computing regions and not transitions for now.
            for (const auto &transition: regOutTransitions)
            {
                const std::string &action = transition.action.first;

                // Skip if we've already processed this action.
                if (processedActions.contains(action))
                    continue;
                processedActions.insert(action);

                // Getting the indices of the transitions corresponding to 'action'.
                // ReSharper disable once CppTooWideScopeInitStatement
                const std::vector<int> &transitionIndices = actionsToTransitionIndices[action];

                // All transitions must lead to a region in setG.
                bool allTransitionsValid = true;
                // Used to keep track of the disabled transitions with action 'action'. If all transitions are disabled, set allTransitionsValid to false.
                size_t disabledTransitions = 0;

                for (const int tIdx: transitionIndices)
                {
                    // A transition must be enabled (its guard must be satisfied), otherwise we ignore it.
                    if (!regOutTransitions[tIdx].isTransitionSatisfied(reg.getClockValuation(), clocksIndices, {}))
                    {
                        disabledTransitions++;
                        continue;
                    }

                    // When set to false, a discrete successor does not belong to setG.
                    bool foundInSetG = true;

                    // We compute discrete successors one transition at a time.
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const std::vector<Region> discSuccs = reg.getImmediateDiscreteSuccessors({ regOutTransitions[tIdx] }, clocksIndices, locationsToInt);

                    // We use a loop here, but since we are computing discrete successors over a single transition, the content of discSuccs is a single region.
                    for (const auto &discSucc: discSuccs)
                        if (!setG.contains(discSucc))
                        {
                            foundInSetG = false;
                            break;
                        }

                    if (!foundInSetG)
                    {
                        allTransitionsValid = false;
                        break;
                    }
                }

                if (disabledTransitions == transitionIndices.size())
                    allTransitionsValid = false;

                if (allTransitionsValid)
                {
                    isRegionValid = true;
                    break;
                }
            }

            // Checking whether the controller is guaranteed to win when the chosen delay is equal to zero (same check as in deltaFilter).
            // Indeed, a discrete predecessor can also be seen as a delay predecessor in which the delay is exactly equal to zero.
            if (isRegionValid && locationsToPlayers.at(reg.getLocation()) != CONTROLLER)
                isRegionValid = delaySuccessorsCheck(reg, setG, checkAllSuccessorsInvariants);

            if (isRegionValid)
            {
#ifdef _OPENMP
                threadLocalRegions[omp_get_thread_num()].push_back(reg);
#else
                threadLocalRegions[0].push_back(reg);
#endif
            }
        }
    }

    mergeResults(threadLocalRegions, filteredRegions);
}


void region::RTSArena::deltaFilter(const regionSet &setG,
                                   const std::vector<RegionPtr> &toProcess,
                                   regionSet &filteredRegions,
                                   const regionSet &intersectionSet,
                                   const bool skipPredecessorsInSetG,
                                   const bool checkAllSuccessorsInvariants) const
{
    constexpr size_t parallelThreshold = PARALLEL_THRESHOLD;

    // Thread-local storage for valid predecessors.
#ifdef _OPENMP
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

#pragma omp parallel for if(toProcess.size() >= parallelThreshold) schedule(dynamic) default(none) \
shared(setG, toProcess, skipPredecessorsInSetG, intersectionSet, checkAllSuccessorsInvariants, threadLocalRegions), \
shared(inTransitions, outTransitions, clocksIndices, locationsToInt, maxConstants, invariants, locationsToPlayers)
    for (int i = 0; i < static_cast<int>(toProcess.size()); i++) // NOLINT(modernize-loop-convert)
    {
        // Getting the current region to process.
        const Region &currentRegion = *toProcess[i];

        // We collect every immediate delay predecessor that we filter later based on the delta filter requirements.
        // ReSharper disable once CppTooWideScopeInitStatement
        const std::vector<Region> delayPreds = currentRegion.getImmediateDelayPredecessors();

        // Processing each immediate delay predecessor to see if it can be inserted in filteredRegions.
        for (const auto &reg: delayPreds)
        {
            if (skipRegion(reg, setG, intersectionSet, skipPredecessorsInSetG))
                continue;

            bool isRegionValid = true;

            // Controller regions only need to pass the skipRegion check.
            // Environment regions must additionally guarantee that all delay successors lead to setG.
            if (locationsToPlayers.at(reg.getLocation()) != CONTROLLER)
                isRegionValid = delaySuccessorsCheck(reg, setG, checkAllSuccessorsInvariants);

            if (isRegionValid)
            {
#ifdef _OPENMP
                threadLocalRegions[omp_get_thread_num()].push_back(reg);
#else
                threadLocalRegions[0].push_back(reg);
#endif
            }
        }
    }

    mergeResults(threadLocalRegions, filteredRegions);
}


bool region::RTSArena::timedReachability(const regionSet &setPhi, regionSet &setG, std::vector<RegionPtr> &toProcess, const int maxIter) const
{
    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start = omp_get_wtime();
#else
    const auto start = std::chrono::high_resolution_clock::now();
#endif

    int currentIteration = 0;
    const int totalStartingRegions = static_cast<int>(setG.size());

    regionSet filteredRegionsOmega{};
    regionSet filteredRegionsDelta{};

    while (currentIteration < maxIter)
    {
        omegaFilter(setG, toProcess, filteredRegionsOmega, setPhi, true, false);
        deltaFilter(setG, toProcess, filteredRegionsDelta, setPhi, true, false);

        if (filteredRegionsOmega.empty() && filteredRegionsDelta.empty())
            break;

        setG.merge(filteredRegionsOmega);
        setG.merge(filteredRegionsDelta);

        toProcess.clear();
        for (const auto &region: setG)
            toProcess.push_back(&region);

        filteredRegionsOmega.clear();
        filteredRegionsDelta.clear();

        currentIteration++;
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


bool region::RTSArena::timedReachability(regionSet &setG, std::vector<RegionPtr> &toProcess, const int maxIter) const
{
    return timedReachability({}, setG, toProcess, maxIter);
}


bool region::RTSArena::timedNextReachability(const regionSet &setPhi, regionSet &setG, std::vector<RegionPtr> &toProcess, const int maxIter) const
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

    regionSet filteredRegionsOmega{};
    regionSet filteredRegionsDelta{};

    while (currentIteration < maxIter)
    {
        omegaFilter(setG, toProcess, filteredRegionsOmega, setPhi, true, false);
        deltaFilter(setG, toProcess, filteredRegionsDelta, setPhi, true, false);

        if (filteredRegionsOmega.empty() && filteredRegionsDelta.empty())
            break;

        setG.merge(filteredRegionsOmega);
        setG.merge(filteredRegionsDelta);

        toProcess.clear();
        for (const auto &region: setG)
            toProcess.push_back(&region);

        filteredRegionsOmega.clear();
        filteredRegionsDelta.clear();

        currentIteration++;
    }

    // Step 2: we perform one additional iteration of omega and delta filters.
    toProcess.clear();
    for (const auto &region: setG)
        toProcess.push_back(&region);

    filteredRegionsOmega.clear();
    filteredRegionsDelta.clear();

    // In this case, we remove the constraints over the intersection set setPhi and put skipPredecessorsInSetG to false.
    omegaFilter(setG, toProcess, filteredRegionsOmega, {}, false, false);
    deltaFilter(setG, toProcess, filteredRegionsDelta, {}, false, false);

    // Step 3: we must reach a delay fixpoint over the previously computed regions, since we operate with immediate delay predecessors only.
    // Note: NOT including setG itself here, since it is unnecessary.
    regionSet &newSetG = filteredRegionsOmega;
    newSetG.merge(filteredRegionsDelta);

    // Since we are reaching a fixpoint over delays, we can remove from newSetG the regions whose location does not coincide with that of an initial region.
    absl::flat_hash_set<int> initialRegionsLocations{};
    for (const auto &region: initialRegions)
        initialRegionsLocations.insert(region.getLocation());
    std::erase_if(newSetG, [&initialRegionsLocations](const auto &region) { return !initialRegionsLocations.contains(region.getLocation()); });

    while (currentIteration < maxIter)
    {
        filteredRegionsDelta.clear();
        toProcess.clear();
        for (const auto &region: newSetG)
            toProcess.push_back(&region);

        deltaFilter(newSetG, toProcess, filteredRegionsDelta, {}, true, false);

        // Removing predecessors that could be contained in setG.
        std::erase_if(filteredRegionsDelta, [&setG](const auto &region) { return setG.contains(region); });

        if (filteredRegionsDelta.empty())
            break;

        newSetG.merge(filteredRegionsDelta);

        currentIteration++;
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

    const bool reachable = std::ranges::any_of(initialRegions, [&newSetG](const auto &region) { return newSetG.contains(region); });

    std::cout << "Total iterations:        " << currentIteration << std::endl;
    std::cout << "Total starting regions:  " << totalStartingRegions << std::endl;
    std::cout << "Total regions in setPhi: " << setPhi.size() << std::endl;
    std::cout << "Total stored regions:    " << setG.size() + newSetG.size() << std::endl;
    std::cout << (reachable ? "VICTORY" : "LOSE") << std::endl;

    return reachable;
}


bool region::RTSArena::timedSafety(regionSet &setG, std::vector<RegionPtr> &toProcess, const int maxIter) const
{
    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start = omp_get_wtime();
#else
    const auto start = std::chrono::high_resolution_clock::now();
#endif

    int currentIteration = 0;
    const int totalStartingRegions = static_cast<int>(setG.size());

    regionSet filteredRegionsOmega{};
    regionSet filteredRegionsDelta{};

    while (currentIteration < maxIter)
    {
        const size_t oldSetGSize = setG.size();

        omegaFilter(setG, toProcess, filteredRegionsOmega, {}, false, true);
        deltaFilter(setG, toProcess, filteredRegionsDelta, {}, false, true);

        // Computing the intersection between regions returned by omega and delta filters and setG.
        filteredRegionsOmega.merge(filteredRegionsDelta);
        std::erase_if(setG, [&filteredRegionsOmega](const auto &region) { return !filteredRegionsOmega.contains(region); });

        // ReSharper disable once CppTooWideScopeInitStatement
        const size_t newSetGSize = setG.size();

        if (oldSetGSize == newSetGSize)
            break;

        toProcess.clear();
        for (const auto &region: setG)
            toProcess.push_back(&region);

        filteredRegionsOmega.clear();
        filteredRegionsDelta.clear();

        currentIteration++;
    }

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


inline bool region::RTSArena::solveGameWithBoxFormula(const cltloc::ast::unaryCLTLocFormula &unaryFormula) const
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


inline bool region::RTSArena::solveGameWithDiamondFormula(const cltloc::ast::unaryCLTLocFormula &unaryFormula) const
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


inline bool region::RTSArena::solveGameWithUntilFormula(const cltloc::ast::binaryCLTLocFormula &binaryFormula) const
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


inline bool region::RTSArena::solveGameWithNextFormula(const cltloc::ast::generalCLTLocFormula &formula) const
{
    // The formula must be of the form: phi UNTIL psi.
    const bool result = std::visit([this]<typename T0>(T0 const &val) -> bool {
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


bool region::RTSArena::solveTimedCLTLocGame(const cltloc::ast::generalCLTLocFormula &formula) const
{
    // Starting the timer for measuring computation.
#ifdef _OPENMP
    const auto start = omp_get_wtime();
#else
    const auto start = std::chrono::high_resolution_clock::now();
#endif

    const bool result = std::visit([this]<typename T0>(T0 const &val) -> bool {
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
