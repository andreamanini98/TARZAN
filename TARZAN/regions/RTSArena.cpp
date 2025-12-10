#include "RTSArena.h"
#include "TARZAN/utilities/file_utilities.h"
#include "TARZAN/exceptions/nestedCLTLocFormula_exception.h"

#define THROW_NESTEDCLTLOC_EXCEPTION


std::vector<region::Region> region::RTSArena::getRegionsFromPureCLTLocFormula(const cltloc::ast::pureCLTLocFormula &formula) const
{
    std::cout << "Now handling the following pure CLTLoc formula: " << formula << std::endl;

    std::vector<Region> res{};

    // Questa dovrà essere la funzione da cui, data una formula cltloc pura, derivi l'insieme di regioni che la caratterizza.
    // Prova a vedere nell'algoritmo di backward reachability se puoi riusare del codice.
    // Ricorda che in una formula pura le locations rappresentano una disgiunzione (predichi su una disgiunzione di locations).
    // I clock constraints invece rappresentano una congiunzione di vincoli.
    // Come scritto sotto, [],[] è un caso degenere in cui tutto lo state space viene considerato, in questo caso si può direttamente decretare il vincitore.
    // Possiamo anche assumere che una tale situazione non si verifichi mai (ma se fai l'algoritmo di generazione delle regioni sufficientemente generale
    // dovresti potere gestire anche quel caso, anche se poi nella realtà non ti serve).

    // In a pure formula, an empty vector of locations or clock constraints means that all possible combinations are admissible.
    // In this way, [],[] may require the computation of the entire state space. We can simplify the analysis in this case (e.g., (BOX, ([],[])) is trivially satisfied???).

    return res;
}


// TODO: in seguito ti servirà potere determinare il tipo di operatore per decidere che algoritmo usare (es. differenza tra box e diamond) per determinare il
//       tipo di algoritmo di games da applicare (safety o reachability). Questa funzione ti conviene farla direttamente in ast.h.
//       Dato che restituisci un vettore di vettori, nel caso di BOX e DIAMOND avrai un solo vettore al suo interno,
//       con UNTIL ne avrai due, okkio a quale si riferisce alla formula sx e dx.
std::vector<std::vector<region::Region>> region::RTSArena::getRegionsFromGeneralCLTLocFormulaWithDepth(const cltloc::ast::generalCLTLocFormula &formula, int depth) const // NOLINT
{
    std::vector<std::vector<Region>> res{};

    std::visit([this, &res, depth]<typename T0>(T0 const &val) {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, bool>)
        {
            // Base case - Boolean value: if true, all possible regions must be generated.
            if (val)
            {
                std::cout << "Calling getRegionsFromGeneralCLTLocFormula from boolean case" << std::endl;

                const cltloc::ast::pureCLTLocFormula pureFormula{};
                auto regions = getRegionsFromPureCLTLocFormula(pureFormula);
                res.push_back(std::move(regions));
            }
        } else if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::pureCLTLocFormula>>)
        {
            // Base case - pure formula: compute regions from this formula.
            std::cout << "Calling getRegionsFromGeneralCLTLocFormula from pure case" << std::endl;

            const auto &pureFormula = val.get();
            auto regions = getRegionsFromPureCLTLocFormula(pureFormula);
            res.push_back(std::move(regions));
        } else if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::unaryCLTLocFormula>>)
        {
            // Recursive case - unary formula.
            std::cout << "Calling getRegionsFromGeneralCLTLocFormula from unary case" << std::endl;

#ifdef THROW_NESTEDCLTLOC_EXCEPTION

            if (depth >= 1)
                throw NestedCLTLocFormulaException("Nested formulae with depth > 1 are not supported.");

#endif

            const auto &unaryFormula = val.get();
            auto tmp = getRegionsFromGeneralCLTLocFormulaWithDepth(unaryFormula.rightFormula, depth + 1);
            res.insert(res.end(), std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
        } else if constexpr (std::is_same_v<T, boost::spirit::x3::forward_ast<cltloc::ast::binaryCLTLocFormula>>)
        {
            // Recursive case - binary formula.
            std::cout << "Calling getRegionsFromGeneralCLTLocFormula from binary case" << std::endl;

#ifdef THROW_NESTEDCLTLOC_EXCEPTION

            if (depth >= 1)
                throw NestedCLTLocFormulaException("Nested formulae with depth > 1 are not supported.");

#endif

            const auto &binaryFormula = val.get();
            auto leftTmp = getRegionsFromGeneralCLTLocFormulaWithDepth(binaryFormula.leftFormula, depth + 1);
            res.insert(res.end(), std::make_move_iterator(leftTmp.begin()), std::make_move_iterator(leftTmp.end()));
            auto rightTmp = getRegionsFromGeneralCLTLocFormulaWithDepth(binaryFormula.rightFormula, depth + 1);
            res.insert(res.end(), std::make_move_iterator(rightTmp.begin()), std::make_move_iterator(rightTmp.end()));
        }
    }, formula.value);

    return res;
}


std::vector<std::vector<region::Region>> region::RTSArena::getRegionsFromGeneralCLTLocFormula(const cltloc::ast::generalCLTLocFormula &formula) const
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
