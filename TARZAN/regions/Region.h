#ifndef REGION_H
#define REGION_H

#include <vector>
#include <boost/dynamic_bitset.hpp>

#include "TARZAN/parser/ast.h"

// TODO: vedere come adattare questo alle regioni delle arene. Secondo me non occorre fare altro, q rende possibile determinare la natura
//       delle locations (controller o environment), ma occorre guardare nella rappresentazione dell'arena. Al limite aggiungi un Bool.

// TODO: implementare nel file ast.h una funzione che restituisce l'idx delle locations iniziali, quindi ti servirà per trovare le locations iniziali.


class Region
{
    /// The location of the region.
    int q{};

    /// The integer values of clocks.
    /// The clocks at a given index of this array correspond to the same clock in the std::vector<std::string> clocks of a Timed Automaton.
    /// @warning The order in bitsets is reversed, e.g., considering (clock, index), h = (x,0)(y,1)(z,2) x0 = (z,2)(y,1)(x,0).
    ///          For this reason, we use indices in a way such that the two representations coincide.
    int *h{};

    /// A vector of bitsets keeping track of the order in which clocks became unbounded.
    /// Index -l corresponds to the front of the deque, while -1 corresponds to the back of the deque.
    std::deque<boost::dynamic_bitset<>> unbounded;

    /// A bitset representing the bounded clocks with no fractional part.
    boost::dynamic_bitset<> x0{};

    /// A vector of bitsets keeping track of the fractional order of bounded clocks.
    /// Index 1 corresponds to the front of the deque, while r corresponds to the back of the deque.
    std::deque<boost::dynamic_bitset<>> bounded;


public:
    /**
     * @brief Creates an initial region (all clocks are in x0 with integer value equal to zero).
     *
     * @param numClocks the number of Timed Automaton clocks from which the region is derived.
     */
    explicit Region(const int numClocks)
    {
        h = static_cast<int *>(malloc(numClocks * sizeof(int)));
        x0.resize(numClocks);
        x0.flip();
    }


    Region(const int q,
           int *h,
           const std::deque<boost::dynamic_bitset<>> &unbounded,
           const boost::dynamic_bitset<> &x0,
           const std::deque<boost::dynamic_bitset<>> &bounded)
        : q(q),
          h(h),
          unbounded(unbounded),
          x0(x0),
          bounded(bounded)
    {}


    // Copy constructor.
    Region(const Region &other)
        : q(other.q),
          unbounded(other.unbounded),
          x0(other.x0),
          bounded(other.bounded)
    {
        // Deep copy the h array.
        const size_t numClocks = x0.size();
        h = static_cast<int *>(malloc(numClocks * sizeof(int)));
        std::memcpy(h, other.h, numClocks * sizeof(int));
    }


    ~Region()
    {
        free(h);
    }


    /**
     * @brief Computes the immediate delay successor of the current region as detailed in our paper.
     *
     * @param maxConstant the maximum constant of the Timed Automaton from which the region is derived.
     * @return a Region immediate delay successor of the current region.
     */
    [[nodiscard]] Region getImmediateDelaySuccessor(int maxConstant) const;


    /**
     * @brief Computes the immediate delay predecessor of the current region as detailed in our paper.
     *
     * @return a std::vector<Region> containing immediate delay predecessors of the current region.
     *         If no predecessors can be computed, returns an empty std::vector.
     */
    [[nodiscard]] std::vector<Region> getImmediateDelayPredecessors() const;


    /**
     * @brief Computes the immediate discrete successor of the current region as detailed in our paper.
     *
     * @param transitions the transitions over which immediate discrete successors must be computed.
     * @return a std::vector<Region> containing immediate discrete successors of the current region.
     *         If no successors can be computed, returns an empty std::vector.
     */
    [[nodiscard]] std::vector<Region> getImmediateDiscreteSuccessors(std::vector<timed_automaton::ast::transition> transitions) const;


    /**
     * @brief Creates a deep copy of this region.
     *
     * @return a new Region object that is a copy of this one.
     */
    [[nodiscard]] Region clone() const
    {
        return { *this };
    }


    /**
     * @return an int representing the total number of clocks considered in this region.
     */
    [[nodiscard]] int getNumberOfClocks() const
    {
        return static_cast<int>(x0.size());
    }


    [[nodiscard]] std::string toString() const;


    // Getters.
    [[nodiscard]] int getLocation() const { return q; }


    // Setters, created only for convenience.
    void set_q(const int q_p) { this->q = q_p; }
    void set_h(int *h_p) { this->h = h_p; }
    void set_unbounded(const std::deque<boost::dynamic_bitset<>> &unbounded_p) { this->unbounded = unbounded_p; }
    void set_x0(const boost::dynamic_bitset<> &x0_p) { this->x0 = x0_p; }
    void set_bounded(const std::deque<boost::dynamic_bitset<>> &bounded_p) { this->bounded = bounded_p; }


    Region &operator=(const Region &other)
    {
        if (this != &other)
        {
            q = other.q;
            unbounded = other.unbounded;
            x0 = other.x0;
            bounded = other.bounded;

            // Deep copy the h array.
            free(h);
            const size_t numClocks = x0.size();
            h = static_cast<int *>(malloc(numClocks * sizeof(int)));
            std::memcpy(h, other.h, numClocks * sizeof(int));
        }
        return *this;
    }
};

#endif //REGION_H
