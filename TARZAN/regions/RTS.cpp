#include "RTS.h"

#include <iostream>
#include <utility>

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
            //std::exit(EXIT_FAILURE);
            std::vector<Region> regionToReturn{};
            regionToReturn.push_back(currentRegion);
            return regionToReturn;
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


std::vector<region::Region> region::RTS::buildRegionGraphBackwards(std::vector<Region> startingRegions) const
{
    std::vector toProcess{ std::move(startingRegions) };
    std::vector result{ startingRegions };

    absl::flat_hash_set<Region, RegionHash> regionsHashMap{};

    // togliere
    unsigned long long int totalregions = 0;

    const auto start = std::chrono::high_resolution_clock::now();
    while (!toProcess.empty())
    {
        std::vector<Region> predecessors{};

        Region currentRegion = toProcess.back();
        toProcess.pop_back();

        // Il codice che vedi qui è servito solo per fare un esempio veloce del flower, dopo va tolto.
        const auto &clockValuation = currentRegion.getClockValuation();

        bool isGoalReached = true;
        for (const auto &[fst, snd]: clockValuation)
            if (fst != 0 || snd == true)
                isGoalReached = false;

        if (currentRegion.getLocation() == 1 && isGoalReached)
        {
            std::cout << "Total number of computed regions: " << totalregions << std::endl;
            std::cout << "MAX CONSTANT IS: " << maxConstant << std::endl;
            std::cout << currentRegion.toString() << std::endl;
            std::cout << "GOAL REGION IS REACHABLE!\n";
            const auto end = std::chrono::high_resolution_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;
            std::cout << "CURRENT REGION: " << currentRegion.toString() << std::endl;
            std::exit(EXIT_FAILURE);
        }

        std::vector<Region> delayPredecessors = currentRegion.getImmediateDelayPredecessors();

        std::vector<transition> transitions = inTransitions[currentRegion.getLocation()];
        std::vector<Region> discretePredecessors = currentRegion.getImmediateDiscretePredecessors(transitions, clocksIndices, locationsToInt, maxConstant);

        totalregions += delayPredecessors.size() + discretePredecessors.size();

        for (const auto &reg: delayPredecessors)
        {
            if (!regionsHashMap.contains(reg))
            {
                result.push_back(reg);
                regionsHashMap.insert(reg);
                toProcess.push_back(reg);
            }
        }
        for (const auto &reg: discretePredecessors)
        {
            if (!regionsHashMap.contains(reg))
            {
                result.push_back(reg);
                regionsHashMap.insert(reg);
                toProcess.push_back(reg);
            }
        }
    }

    return result;
}
