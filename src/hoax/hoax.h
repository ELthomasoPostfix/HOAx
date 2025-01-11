#ifndef HOAX_H
#define HOAX_H

#include "utils.h"
#include <assert.h>
#include <utility>
#include <climits>
#include <vector>
#include <cmath>
#include <set>
#include <spot/parseaut/public.hh>
#include <spot/twaalgos/game.hh>

namespace hoax {

    /** The player 'index' for the even player, player 0, Eve. */
    #define PEVEN 0
    /** The player 'index' for the odd player, player 1, Adam. */
    #define PODD  1

    /* Define spot named property constants. */
    #define PROP_SPOT_STATE_WINNER "state-winner"
    #define PROP_SPOT_SYNTH_OUTPUT "synthesis-outputs"
    #define PROP_SPOT_STRAT "strategy"
    #define PROP_SPOT_STATE_PLAYER "state-player"

    /* Define hoax named property constants. */
    #define PROP_HOAX_STATE_WINNER "state-winner-hoax"
    #define PROP_HOAX_PRIOR "priority-hoax"

    /** An interface for expanding a parity automaton into a parity arena.

        In this context, we use "parity automaton" to mean a spot TwA that only
        has "odd player" states and propositional formulas labeling its
        transitions.

        A "parity arena" is then the explicit expansion of this automaton by
        adding "even player" states, and new transitions to the translation retain
        the meaning of the original automaton.

        The size of this automaton is exponential in function of the number of
        uncontrollable APs. Refer to spot's "synthesis-outputs" header, which
        encodes the eHOA "controllable-AP" header, for how to access the
        controllable APs.
    */
    struct HOAxParityTwA {
    public:
        /** The TwA that was transformed to generate the expanded TwA. */
        spot::twa_graph_ptr src;
        /** The expanded TwA, where the even player states have been made explicit. */
        spot::twa_graph_ptr exp;

        /** The deadline's start horizon for this parity game. */
        const clock_t start;
        /** A reference deadline to hint when execution should be quit early. */
        const clock_t deadline;

    public:
        /** Expands the given parity automaton.

            @param[in] aut The parity game to expand
            @param[in] start_t The deadline's start horizon for this parity game
            @param[in] deadline_t If this deadline is exceeded, then quit early
        */
        HOAxParityTwA(const spot::twa_graph_ptr aut, const clock_t &start_t, const clock_t &deadline_t);

        /** Solve a "parity min/max odd" game.

            i.e. the actual arena may have any acceptance condition,
            but the algorithm checks if the odd player can win the
            game from the initial state, given the additional min/max
            parity parameter.

            @param[in] parity_max If true, then solve for the "parity max" condition.
                                  Else solve for the "parity min" condition.
            @return true iff. the "odd" player wins from the initial state.
                    false else, i.e. the "even" player wins from the initial state.
                    Since the "odd" player is represented by "1" or "true" and
                    the "even" player is represented by "0" or "false",
                    the return value is effectively the winning player.
         */
        bool solve_parity_game(const bool parity_max) const;

        /** Overwrite the "state-names" named spot prop of the parity arena. */
        void set_state_names();

        /** Throw a `std::runtime_error` iff. `clock() > this->deadline`. */
        void assert_deadline() const;

    private:
        /* Get the set of all state numbers. */
        std::set<int> get_all_states() const;

        /* Get the set of state numbers for "even player states". */
        std::set<int> get_even_states() const;

        /* Get the set of state numbers for "odd player states". */
        std::set<int> get_odd_states() const;
    };

    /** Zielonka's algorithm for solving a parity game.

        @param[out] W0 The winning set of player 0, Eve.
        @param[out] W1 The winning set of player 1, Adam.
        @param[in] vertices All vertices to parition into W0 and W1.
        @param[in] vertices_even The "even player" subset of vertices
        @param[in] aut The parity game arena.
        @param[in] parity_max If true, then solve for the "parity max" condition.
                            Else solve for the "parity min" condition.
    */
    void zielonka(
        std::set<int> *W0,
        std::set<int> *W1,
        const std::set<int> *vertices,
        const std::set<int> *vertices_even,
        const HOAxParityTwA &aut,
        const bool parity_max);

    /** Compute the attractor set for the given player and arena.

        This is a loop-based implementation of the attractor set's recursive
        definition.

        Note that the sets "odd" and "even" vertices passed to this function
        should be subsets of, but not necessarily equal to, the complete sets
        of all "odd" resp. "even" states in the entire automaton.

        @param[out] attr The attractor set
        @param[in] vertices_all The set of all states in the parity arena
                                which to include in the attractor computation
        @param[in] vertices_odd The set of "odd player states" in the parity arena
                                which to include in the attractor computation
        @param[in] vertices_odd The set of "even player states" in the parity arena
                                which to include in the attractor computation
        @param[in] aut The parity arena
        @param[in] T The vertices from which to start the attractor computation
        @param[in] i The player for whom to compute the attractor set
    */
    void attractor(
        std::set<int> &attr,
        const std::set<int> &vertices_all,
        const std::set<int> &vertices_odd,
        const std::set<int> &vertices_even,
        const HOAxParityTwA &aut,
        const std::set<int> &T,
        const unsigned int i);

    /** The parity game priority function for a single edge.

        Each edge in spot specifies the acceptance sets of which it is a part.
        The priority function is derived from the acceptance set's indexes.

        @param[in] mark The acceptance sets membership bitvector of an edge
        @param[in] parity_max If true, then return the highest priority.
                              If false, then return the lowest priority.
        @return The edge priority value.
    */
    int priority(const spot::acc_cond::mark_t &mark, const bool parity_max);

    /** The parity game priority function for a single state.

        Implicitly assumes transition-based acceptance. This is possible because
        spot always encodes acceptance set membership through the transitions.

        @pre The given state may not be a sink node, it must have outgoing edges.
        @param[in] aut The TwA automaton to which the state belongs.
        @param[in] state The state to compute the priority for.
        @param[in] parity_max If true, then return the highest priority.
                            If false, then return the lowest priority.
        @return The state priority value.
    */
    int priority(const spot::twa_graph_ptr aut, const unsigned int state, const bool parity_max);
}

#endif