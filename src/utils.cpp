#include "utils.h"

bool contains(const std::set<int> *set, const int value) {
    return set->find(value) != set->end();
}

std::ostream &operator<<(std::ostream &os, const HOAxState &state) {
    os << "{" << state.first << ", " << state.second << "}";
    return os;
}

bool operator<(const HOAxState &s1, const HOAxState &s2) {
    // Resort to comparing bdd IDs to break equality ties.
    return (s1.first < s2.first) || (s1.second.id() < s2.second.id());
}

bdd bdd_variables(const bdd &r) {
    return bdd_variables_(r, true);
}

bdd bdd_variablescomp(const bdd &r) {
    return bdd_variables_(r, false);
}

bdd bdd_variables_(const bdd &r, const bool var_should_appear) {
    int *vprof = bdd_varprofile(r);

    /* Use a boolean rule: T & a = a */
    bdd conjunction = bddtrue;
    for (int i = 0; i < bdd_varnum(); i++) {
        bool var_appears = vprof[i] > 0;
        if (var_appears == var_should_appear)
            conjunction &= bdd_ithvar(i);
    }

    if (vprof) free(vprof);    // spot does not free this mem.
    return conjunction;
}

void bdd_var_indexes(const bdd &r, int **indexes, unsigned int *size) {
    /* Can only pass up the assigned array if the pointer is valid. */
    assert(indexes != nullptr);
    assert(size != nullptr);

    int *vprof = bdd_varprofile(r);

    /* First pass: find out how many variables appear. */
    unsigned int count = 0;
    for (int i = 0; i < bdd_varnum(); i++)
        count += vprof[i] > 0;

    *size = count;
    /* Prevent memory allocation if there are no variables. */
    if (count == 0) return;
    *indexes = new int[count];

    /* Second pass: fill the array. */
    for (int i = 0; i < bdd_varnum(); i++)
        if (vprof[i] > 0)
            **indexes = i;

    if (vprof) free(vprof);    // spot does not free this mem.
}
