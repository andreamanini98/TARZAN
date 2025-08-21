#ifndef REGION_H
#define REGION_H

#include <vector>
#include <bitset>
#include <boost/dynamic_bitset.hpp>

// TODO: vedere come adattare questo alle regioni delle arene. Secondo me non occorre fare altro, q rende possibile determinare la natura
//       delle locations (controller o environment), ma occorre guardare nella rappresentazione dell'arena. Al limite aggiungi un Bool.


class Region {
    // The location of the region. TODO: magari una stringa è più semplice da gestire? Ti evita tutto l'encoding delle locations, ma dovrebbe richiedere più bit.
    int q{};

    // The integer values of clocks.
    std::vector<int> h{};

    // A vector of bitsets keeping track of the fractional order of bounded clocks.
    std::vector<boost::dynamic_bitset<>> bounded;

    // A vector of bitsets keeping track of the order in which clocks became unbounded.
    std::vector<boost::dynamic_bitset<>> unbounded;

    // A bitset representing the bounded clocks with no fractional part.
    boost::dynamic_bitset<> x0{};


public:
    /**
     * @brief Creates an initial region (all clocks are in x0 with integer value equal to zero).
     *
     * @param numClocks the number of Timed Automaton clocks from which the region is derived.
     */
    explicit Region(const size_t numClocks)
    {
        h.resize(numClocks);
        x0.resize(numClocks);
        x0.flip();
    }


    Region(const int q,
           const std::vector<int> &h,
           const std::vector<boost::dynamic_bitset<>> &bounded,
           const std::vector<boost::dynamic_bitset<>> &unbounded,
           const boost::dynamic_bitset<> &x0)
        : q(q),
          h(h),
          bounded(bounded),
          unbounded(unbounded),
          x0(x0)
    {
    }


    /**
     * @brief Computes the delay successor of the current region as detailed in our paper.
     *
     * @param maxConstant the maximum constant of the Timed Automaton from which the region is derived.
     * @return a Region delay successor of the current region.
     */
    Region getDelaySuccessor(int maxConstant);


    /**
     * @brief Computes the delay predecessor of the current region as detailed in our paper.
     *
     * @param maxConstant the maximum constant of the Timed Automaton from which the region is derived.
     * @return a Region delay predecessor of the current region.
     */
    Region getDelayPredecessor(int maxConstant);

    // TODO: per i discrete, occorre passare anche le transizioni del TA (oppure ricavare qualcosa prima)


    [[nodiscard]] std::string toString() const;
};

#endif //REGION_H
