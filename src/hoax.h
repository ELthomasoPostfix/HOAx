#ifndef HOAX_H
#define HOAX_H

#include "utils.h"
#include <functional>
#include <assert.h>
#include <utility>
#include <vector>
#include <climits>
#include <set>

#define PEVEN 0
/** The player 'index' for the even player, player 0, Eve. */
#define PODD  1
/** The player 'index' for the odd player, player 1, Adam. */

/** Zielonka's algorithm for solving a parity game.

    @param[out] W0 The winning set of player 0, Eve.
    @param[out] W1 The winning set of player 1, Adam.
 */
void zielonka(
    std::set<int> *W0,
    std::set<int> *W1,
    const std::vector<int> *vertices,
    const std::vector<int> *vertices_even,
    const std::vector<int> *edges,
    std::function<int(int)> priority_fun);

void attractor(
    std::set<int> *attr,
    const std::vector<int> *vertices_odd,
    const std::vector<int> *vertices_even,
    const std::vector<int> *edges,
    const std::set<int> *T,
    const int k,
    const int i);

#endif