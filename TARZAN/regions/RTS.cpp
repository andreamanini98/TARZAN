#include "RTS.h"

#include <iostream>

#include "absl/container/flat_hash_set.h"


// TODO: quando calcoli una regione e questa è di classe U, non ti interessa calcolare nuovamente il successore delay (sarà sempre quello), quindi secondo me
//       puoi anche non calcolare un successore di una regione di classe U e di conseguenza non mettere auto-anelli nel RTS, tanto sono già sottintesi.

// TODO: non ti dovrebbe servire salvarti tutto il RTS per visualizzarlo graficamente. Se dovesse servirti, potresti mettere direttamente dei puntatori nelle regioni
//       che puntano ai loro successori (uno per i delay e uno per i discreti). Salva una regione in uno di questi puntatori (la reference ovviamente) solo quando
//       sei sicuro che non ne hai già trovata un'altra uguale; in caso contrario, salva direttamente nel puntatore la regione già calcolata.

// TODO: per visualizzare in modo più carino il RTS (se te lo crei) usa graphviz.

// TODO: implementare un vero BFS.

std::vector<region::Region> region::RTS::buildRegionGraphForeword() const
{
    std::vector toProcess{ getInitialRegions() };
    std::vector result{ getInitialRegions() };

    absl::flat_hash_set<Region, RegionHash> regionsHashMap{};

    // togliere
    unsigned long long int totalregions = 0;

    const auto start = std::chrono::high_resolution_clock::now();
    while (!toProcess.empty())
    {
        std::vector<Region> successors{};

        Region currentRegion = toProcess.back();
        toProcess.pop_back();

        // Il codice che vedi qui è servito solo per fare un esempio veloce del flower, dopo va tolto.
        //std::cout << currentRegion.toString() << std::endl;
        if (currentRegion.getLocation() == 0)
        {
            std::cout << "Total number of computed regions: " << totalregions << std::endl;
            std::cout << "MAX CONSTANT IS: " << maxConstant << std::endl;
            std::cout << currentRegion.toString() << std::endl;
            std::cout << "GOAL REGION IS REACHABLE!\n";
            const auto end = std::chrono::high_resolution_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        Region delaySuccessor = currentRegion.getImmediateDelaySuccessor(maxConstant);

        std::vector<transition> transitions = outTransitions[currentRegion.getLocation()];
        std::vector<Region> discreteSuccessors = currentRegion.getImmediateDiscreteSuccessors(transitions, clocksIndices, locationsToInt);

        totalregions += discreteSuccessors.size() + 1;

        if (!regionsHashMap.contains(delaySuccessor))
        {
            regionsHashMap.insert(delaySuccessor);
            toProcess.push_back(delaySuccessor);
        }
        for (const auto &reg: discreteSuccessors)
        {
            if (!regionsHashMap.contains(reg))
            {
                regionsHashMap.insert(reg);
                toProcess.push_back(reg);
            }
        }
    }

    return result;
}
