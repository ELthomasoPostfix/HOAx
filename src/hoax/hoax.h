#ifndef HOAX_H
#define HOAX_H

#include "utils.h"
#include <functional>
#include <assert.h>
#include <utility>
#include <vector>
#include <climits>
#include <cmath>
#include <set>
#include <spot/parseaut/public.hh>
#include <spot/twaalgos/game.hh>

#define PEVEN 0
/** The player 'index' for the even player, player 0, Eve. */
#define PODD  1
/** The player 'index' for the odd player, player 1, Adam. */

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

public:
    /** Expands the given parity automaton. */
    HOAxParityTwA(const spot::twa_graph_ptr aut);

    /** Overwrite the "state-names" named spot prop of the parity arena. */
    void set_state_names();
};

/** Zielonka's algorithm for solving a parity game.

    @param[out] W0 The winning set of player 0, Eve.
    @param[out] W1 The winning set of player 1, Adam.
    @param[in] parity_max If true, then solve for the "parity max" condition.
                          Else solve for the "parity min" condition.
 */
void zielonka(
    std::set<int> *W0,
    std::set<int> *W1,
    const std::set<int> *vertices,
    const std::set<int> *vertices_even,
    const spot::twa_graph_ptr aut,
    const bool parity_max);

void attractor(
    std::set<int> *attr,
    const std::set<int> *vertices_odd,
    const std::set<int> *vertices_even,
    const spot::twa_graph_ptr aut,
    const std::set<int> *T,
    const int k,
    const int i);

/** The parity game priority function for a single edge.

    Each edge in spot specifies the acceptance sets of which it is a part.
    The priority function is derived from the acceptance set's indexes.

    @pre The edge must be part of at least one acceptance set.
    @param[in] mark The acceptance sets membership bitvector of an edge
    @param[in] parity_max If true, then return the highest priority.
                          If false, then return the lowest priority.
    @return The edge priority value.
*/
unsigned int priority(const spot::acc_cond::mark_t &mark, const bool parity_max);

/** The parity game priority function for a single state.

    Implicitly assumes transition-based acceptance. This is possible because
    spot always encodes acceptance set membership through the transitions.

    @pre All of the outgoing edges of the state must be part of at least one
         acceptance set.
    @pre The given state may not be a sink node, it must have outgoing edges.
    @param[in] aut The TwA automaton to which the state belongs.
    @param[in] state The state to compute the priority for.
    @param[in] parity_max If true, then return the highest priority.
                          If false, then return the lowest priority.
    @return The state priority value.
*/
unsigned int priority(const spot::twa_graph_ptr aut, const unsigned int state, const bool parity_max);

#endif