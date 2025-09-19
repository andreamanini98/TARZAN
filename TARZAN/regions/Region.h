#ifndef REGION_H
#define REGION_H

#include <vector>
#include <boost/dynamic_bitset.hpp>

#include "TARZAN/parser/ast.h"

using transition = timed_automaton::ast::transition;

// TODO: vedere come adattare questo alle regioni delle arene. Secondo me non occorre fare altro, q rende possibile determinare la natura
//       delle locations (controller o environment), ma occorre guardare nella rappresentazione dell'arena. Al limite aggiungi un Bool.

// TODO: Quando farai l'algoritmo per trovare il region graph, devi vedere se dopo una discrete puoi subito trovare una discrete (almeno per i successors) =>
//       comportamento zeno (non è escluso).


namespace region
{
    class Region
    {
        /// The location of the region.
        int q{};

        /**
         * The integer values of clocks.
         * The clocks at a given index of this array correspond to the same clock in the std::vector<std::string> clocks of a Timed Automaton.
         *
         * @warning The order in bitsets is reversed, e.g., considering (clock, index), h = (x,0)(y,1)(z,2) x0 = (z,2)(y,1)(x,0).
         *          For this reason, we use indices in a way such that the two representations coincide.
         */
        int *h{};

        /**
         * A vector of bitsets keeping track of the order in which clocks became unbounded.
         * Index -l corresponds to the front of the deque, while -1 corresponds to the back of the deque.
         */
        std::deque<boost::dynamic_bitset<>> unbounded;

        /// A bitset representing the bounded clocks with no fractional part.
        boost::dynamic_bitset<> x0{};

        /**
         * A vector of bitsets keeping track of the fractional order of bounded clocks.
         * Index 1 corresponds to the front of the deque, while r corresponds to the back of the deque.
         */
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


        /**
         * @brief Creates an initial region (all clocks are in x0 with integer value equal to zero) with location q.
         *
         * @param numClocks the number of Timed Automaton clocks from which the region is derived.
         * @param q the location of the region.
         */
        Region(const int numClocks, const int q) : q(q)
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


        Region(const int q,
               const std::vector<int> &H,
               const std::deque<boost::dynamic_bitset<>> &unbounded,
               const boost::dynamic_bitset<> &x0,
               const std::deque<boost::dynamic_bitset<>> &bounded)
            : q(q),
              unbounded(unbounded),
              x0(x0),
              bounded(bounded)
        {
            const int numOfClocks = static_cast<int>(H.size());

            h = static_cast<int *>(malloc(numOfClocks * sizeof(int)));

            for (int i = 0; i < numOfClocks; i++)
                h[i] = H[i];
        }


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
         * @brief Computes the clock valuation of a region.
         *
         * The clock valuation consists in a std::vector of pairs. Each index of the vector corresponds to the index of a clock (as clocks correspond to
         * the indices of the clocks Timed Automaton vector). If a clock is not in x0, then that clock has a fractional part that is greater than zero.
         *
         * @return a std::vector containing the integer value of a clock and whether the clock has a fractional aprt greater than zero.
         */
        [[nodiscard]] std::vector<std::pair<int, bool>> getClockValuation() const;


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
         * @param clockIndices the indices of the clocks as they appear in the clocks vector of a Timed Automaton.
         * @param locationsAsIntMap a std::unordered_map associating an integer with each string name.
         * @return a std::vector<Region> containing immediate discrete successors of the current region.
         *         If no successors can be computed, returns an empty std::vector.
         *
         * @warning The transitions parameter must contain all and only the transitions exiting from the location of the region.
         *          To provide such transitions, the getOutTransitions() function of a Timed Automaton can be used.
         * @warning To provide the clockIndices parameter, the getClocksIndices() function of a Timed Automaton can be used.
         * @warning To provide the locationsAsIntMap, the mapLocationsToInt() function of a Timed Automaton can be used.
         */
        [[nodiscard]] std::vector<Region> getImmediateDiscreteSuccessors(const std::vector<transition> &transitions,
                                                                         const std::unordered_map<std::string, int> &clockIndices,
                                                                         const std::unordered_map<std::string, int> &locationsAsIntMap) const;


        /**
         * @brief Computes the ordered partitions of X while preserving the order of the clock sets corresponding to indices i to j, where i <= j.
         *
         * @param partBounded used to indicate the indices i...j, since in the paper we consider either unbounded or bounded clocks.
         * @param X the set of clocks for which we compute all ordered partitions.
         * @param cMax the maximum constant of the original Timed Automaton.
         * @param H a vector containing the values of the clocks (indices are computed as usual for Timed Automata, see the ast.h file).
         * @param notInX0 a bitset where the i-th bit is set to 1 if the i-th clock must not be contained in x0.
         * @return a std::vector of Regions obtained by computing the ordered partitions of X while preserving the order of sets from i to j.
         */
        // TODO: H in questo caso rappresenta i valori dei clock che devono essere assegnati alle nuove regioni, vedere se va bene trattarlo
        //       così quando si deve gestire per i bounded il caso in cui un clock sia uguale a cmax.
        [[nodiscard]] std::vector<Region> permRegs(bool partBounded,
                                                   const boost::dynamic_bitset<> &X,
                                                   int cMax,
                                                   const std::vector<int> &H,
                                                   const boost::dynamic_bitset<> &notInX0) const;


        // TODO: comment this function.
        // TODO: quando devi calcolare H per passarla a permRegs e sei nel caso bounded, se hai vincoli del tipo:
        //       x < 3 : devi arrivare al massimo a 2, ossia x può assumere valori interi 0, 1, 2 (non può essere 3 se deve essere minore di 3)
        //       x > 3 : devi partire da 3, però in questo caso metterai x nel bitset di clock che non devono andare in X0
        [[nodiscard]] std::vector<Region> getImmediateDiscretePredecessors(const std::vector<transition> &transitions,
                                                                           const std::unordered_map<std::string, int> &clockIndices,
                                                                           const std::unordered_map<std::string, int> &locationsAsIntMap) const;


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


        bool operator==(const Region &other) const
        {
            if (this == &other)
                return true;
            if (q != other.q)
                return false;
            if (x0 != other.x0)
                return false;
            if (unbounded != other.unbounded)
                return false;
            if (bounded != other.bounded)
                return false;

            const size_t numClocks = x0.size();
            for (size_t i = 0; i < numClocks; ++i)
                if (h[i] != other.h[i]) return false;

            return true;
        }


        bool operator!=(const Region &other) const
        {
            return !(*this == other);
        }


        friend struct RegionHash;
    };


    /// Hash function for Region.
    struct RegionHash
    {
        std::size_t operator()(const Region &region) const
        {
            std::size_t seed = 0;

            // Hash the location.
            hash_combine(seed, region.getLocation());

            // Hash the integer array h.
            const int numClocks = region.getNumberOfClocks();
            for (int i = 0; i < numClocks; i++)
                hash_combine(seed, region.h[i]);

            // Hash the x0 bitset.
            hash_combine(seed, hash_bitset(region.x0));

            // Hash the unbounded deque (include size for order).
            hash_combine(seed, region.unbounded.size());
            for (const auto &bitset: region.unbounded)
                hash_combine(seed, hash_bitset(bitset));

            // Hash the bounded deque (include size for order).
            hash_combine(seed, region.bounded.size());
            for (const auto &bitset: region.bounded)
                hash_combine(seed, hash_bitset(bitset));

            return seed;
        }


    private:
        /**
         * @brief Combines two hashes using golden ratio hashing.
         *
         * @tparam T the class of the new element for which we are combining the hash.
         * @param seed an existing hash value.
         * @param v an element whose hash value must be combined with seed.
         */
        template<class T>
        static void hash_combine(std::size_t &seed, const T &v)
        {
            std::hash<T> hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }


        /**
         * @brief Hashes a boost::dynamic_bitset exploiting its string representation.
         *
         * @param bitset the boost::dynamic_bitset to hash.
         * @return the hash of bitset.
         */
        static std::size_t hash_bitset(const boost::dynamic_bitset<> &bitset)
        {
            std::size_t seed = 0;
            hash_combine(seed, bitset.size());

            std::string bit_string;
            boost::to_string(bitset, bit_string);
            hash_combine(seed, std::hash<std::string>{}(bit_string));

            return seed;
        }
    };
}

#endif //REGION_H
