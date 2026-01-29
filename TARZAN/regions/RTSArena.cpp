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
        std::erase_if(res, [this](const Region &reg) {
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


inline bool region::RTSArena::piEnvironment(const Region &reg, const regionSet &setG, const absl::flat_hash_map<std::string, bool> &validActions) const
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
            everyOutTransitionIsInSetG(oldDelaySucc, setG, actionName, isRegionValid, atLeastOneDiscreteSuccessor);
        else
        {
            while (oldDelaySucc != newDelaySucc)
            {
                everyOutTransitionIsInSetG(oldDelaySucc, setG, actionName, isRegionValid, atLeastOneDiscreteSuccessor);

                // If the region is not valid, by the pi_e condition we can stop checking the sequence of delay successors and try the next action.
                if (!isRegionValid)
                    break;

                oldDelaySucc = newDelaySucc;
                newDelaySucc = oldDelaySucc.getImmediateDelaySuccessor(maxConstants);
            }

            // The if is needed to skip the computation of everyOutTransitionIsInSetG is isRegionValid is already false.
            if (isRegionValid)
                everyOutTransitionIsInSetG(oldDelaySucc, setG, actionName, isRegionValid, atLeastOneDiscreteSuccessor);
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
                                                     std::vector<std::vector<Region>> &threadLocalRegions,
                                                     const absl::flat_hash_map<std::string, bool> &validActions) const
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool isCurrentPlayerController = locationsToPlayers.at(reg.getLocation()) == CONTROLLER;

    if (isCurrentPlayerController ? piController(validActions) : piEnvironment(reg, setG, validActions))
    {
#ifdef _OPENMP
        threadLocalRegions[omp_get_thread_num()].push_back(reg);
#else
        threadLocalRegions[0].push_back(reg);
#endif
    }
}


void region::RTSArena::piFilter(const regionSet &setG,
                                const std::vector<RegionPtr> &toProcess,
                                regionSet &filteredRegions,
                                const regionSet &intersectionSet,
                                bool skipPredecessorsInSetG,
                                bool checkAllSuccessorsInvariants) const
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
shared(inTransitions, outTransitions, clocksIndices, locationsToInt, maxConstants, invariants, initialRegions, locationsToPlayers)
    for (int i = 0; i < static_cast<int>(toProcess.size()); i++) // NOLINT(modernize-loop-convert)
    {
        // Getting the current region to process and its incoming transitions.
        const Region &currentRegion = *toProcess[i];
        const std::vector<transition> &currTransitions = inTransitions[currentRegion.getLocation()];

        // We collect every discrete predecessor that we filter later based on the pi conditions.
        // ReSharper disable once CppTooWideScopeInitStatement
        const std::vector<Region> discPreds = currentRegion.getImmediateDiscretePredecessors(currTransitions, clocksIndices, locationsToInt, maxConstants);

        // Processing each discrete predecessor to see if it can be inserted in filteredRegions.
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

                if (regUnderAnalysis.hasAtLeastOneDiscretePredecessor(inTransitions[regUnderAnalysis.getLocation()], clocksIndices))
                    collectLegalRegionByPi(regUnderAnalysis, setG, threadLocalRegions, validActions);
            }

            // Checking the case in which a region with no delay predecessors is either initial or has an incoming discrete transition (has a discrete predecessor).
            for (const auto &regStillToProcess: regionsStillToProcess)
            {
                const bool isRegionInitial = std::ranges::find(initialRegions, regStillToProcess) != initialRegions.end();
                // ReSharper disable once CppTooWideScopeInitStatement
                const bool hasDiscPreds = regStillToProcess.hasAtLeastOneDiscretePredecessor(inTransitions[regStillToProcess.getLocation()], clocksIndices);

                if (isRegionInitial || hasDiscPreds)
                    collectLegalRegionByPi(regStillToProcess, setG, threadLocalRegions, validActions);
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

    regionSet filteredRegions{};

    while (currentIteration < maxIter)
    {
        piFilter(setG, toProcess, filteredRegions, setPhi, true, false);

        if (filteredRegions.empty())
            break;

        setG.merge(filteredRegions);

        toProcess.clear();
        for (const auto &region: setG)
            toProcess.push_back(&region);

        filteredRegions.clear();

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

    regionSet filteredRegions{};

    while (currentIteration < maxIter)
    {
        piFilter(setG, toProcess, filteredRegions, setPhi, true, false);

        if (filteredRegions.empty())
            break;

        setG.merge(filteredRegions);

        toProcess.clear();
        for (const auto &region: setG)
            toProcess.push_back(&region);

        filteredRegions.clear();

        currentIteration++;
    }

    // Step 2: we perform one additional iteration of omega and delta filters.
    toProcess.clear();
    for (const auto &region: setG)
        toProcess.push_back(&region);

    filteredRegions.clear();

    // In this case, we remove the constraints over the intersection set setPhi and put skipPredecessorsInSetG to false.
    piFilter(setG, toProcess, filteredRegions, {}, false, false);
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

    regionSet filteredRegions{};

    while (currentIteration < maxIter)
    {
        const size_t oldSetGSize = setG.size();

        piFilter(setG, toProcess, filteredRegions, {}, false, true);

        // Computing the intersection between regions returned by omega and delta filters and setG.
        std::erase_if(setG, [&filteredRegions](const auto &region) { return !filteredRegions.contains(region); });

        if (oldSetGSize == setG.size())
            break;

        toProcess.clear();
        for (const auto &region: setG)
            toProcess.push_back(&region);

        filteredRegions.clear();

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
