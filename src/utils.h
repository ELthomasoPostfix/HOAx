#ifndef HOAX_UTILS_H
#define HOAX_UTILS_H

#include <utility>
#include <set>
#include <spot/twaalgos/game.hh>

/** A type alias for states used in solving parity game realizability. */
#define HOAxState std::pair<const unsigned int, const bdd>

/** A wrapper to check membership of a value in a set. */
bool contains(const std::set<int> *set, const int value);

std::ostream &operator<<(std::ostream &os, const HOAxState &state);

/** Return true iff. s1 < s2. */
bool operator<(const HOAxState &s1, const HOAxState &s2);

/** Find the conjunction of all variables that appear in the bdd.

    Defaults to `bddtrue` if no variables are used.

    For example, given 5 variables: a, b, c, d, e
        a | (e & c) | !a    ==>    a & c & e

    @param[in] r The bdd to examine
    @return The conjunction bdd
 */
bdd bdd_variables(const bdd &r);

/** Find the conjunction of all variables that do not appear in the bdd.

    Defaults to `bddtrue` if all variables are used.

    For example, given 5 variables: a, b, c, d, e
        a | (e & c) | !a    ==>    b & d

    @param[in] r The bdd to examine
    @return The conjunction bdd
 */
bdd bdd_variablescomp(const bdd &r);

/** Find the conjunction of all variables that (do not) appear in the bdd.

    Defaults to `bddtrue` if the condition is never satisfied.

    @param[in] r The bdd to examine
    @param[in] var_should_appear If true, then add variables that do appear.
                                 If false, then add variables that do not.
    @return The conjunction bdd
*/
bdd bdd_variables_(const bdd &r, const bool var_should_appear);

/** Find the variable index of each of the variables that appear in the bdd.

    If the expression contains no variables, then no memory will be assigned.
    The memory allocated for the indexes array must be freed by the caller.

    @pre The index array pointer (int**) must not be a nullptr.
    @pre The size pointer must not be a nullptr.
    @post The size value is always assigned.
    @param[in] r The bdd to examine
    @param[out] indexes The pointer that the created array is assigned to
    @param[out] size The pointer that the array size is assigned to
*/
void bdd_var_indexes(const bdd &r, int **indexes, unsigned int *size);

#endif
