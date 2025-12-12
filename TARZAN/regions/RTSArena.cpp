#include "RTSArena.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/exceptions/nestedCLTLocFormula_exception.h"
#include "TARZAN/utilities/function_utilities.h"

#define RTSARENA_DEBUG
#define THROW_NESTEDCLTLOC_EXCEPTION


std::unordered_set<region::Region, region::RegionHash> region::RTSArena::getRegionsFromPureCLTLocFormula(const cltloc::ast::pureCLTLocFormula &formula) const
{
#ifdef RTSARENA_DEBUG

    std::cout << "Now handling the following pure CLTLoc formula: " << formula << std::endl;

#endif

    const int numOfClocks = static_cast<int>(clocksIndices.size());
    return Region::generateRegionsFromConstraints(formula.locations, formula.clockConstraints, clocksIndices, locationsToInt, maxConstants, numOfClocks);
}


// TODO: in seguito ti servirà potere determinare il tipo di operatore per decidere che algoritmo usare (es. differenza tra box e diamond) per determinare il
//       tipo di algoritmo di games da applicare (safety o reachability). Questa funzione ti conviene farla direttamente in ast.h.
//       Dato che restituisci un vettore di vettori, nel caso di BOX e DIAMOND avrai un solo vettore al suo interno,
//       con UNTIL ne avrai due, attenzione a quale si riferisce alla formula sx e dx.

// TODO: per ora l'implementazione corrente va bene perchè non hai annidamenti e quindi hai al più due vettori nel vettore esterno risultante.
//       vedere di trovare il modo di rendere il tutto più generale qualora vengano resi disponibili livelli di annidamento nelle formule.
std::vector<std::unordered_set<region::Region, region::RegionHash>> region::RTSArena::getRegionsFromGeneralCLTLocFormulaWithDepth(
    const cltloc::ast::generalCLTLocFormula &formula, int depth) const // NOLINT
{
    std::vector<std::unordered_set<Region, RegionHash>> res{};

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

            std::vector<std::unordered_set<Region, RegionHash>> tmp = getRegionsFromGeneralCLTLocFormulaWithDepth(unaryFormula.rightFormula, depth + 1);
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

            std::vector<std::unordered_set<Region, RegionHash>> leftTmp = getRegionsFromGeneralCLTLocFormulaWithDepth(binaryFormula.leftFormula, depth + 1);
            res.reserve(res.size() + leftTmp.size());
            res.insert(res.end(), std::make_move_iterator(leftTmp.begin()), std::make_move_iterator(leftTmp.end()));

            std::vector<std::unordered_set<Region, RegionHash>> rightTmp = getRegionsFromGeneralCLTLocFormulaWithDepth(binaryFormula.rightFormula, depth + 1);
            res.reserve(res.size() + rightTmp.size());
            res.insert(res.end(), std::make_move_iterator(rightTmp.begin()), std::make_move_iterator(rightTmp.end()));
        } else
            throw std::logic_error("Unhandled formula type in getRegionsFromGeneralCLTLocFormulaWithDepth.");
    }, formula.value);

    return res;
}


std::vector<std::unordered_set<region::Region, region::RegionHash>> region::RTSArena::getRegionsFromGeneralCLTLocFormula(
    const cltloc::ast::generalCLTLocFormula &formula) const
{
    return getRegionsFromGeneralCLTLocFormulaWithDepth(formula, 0);
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
