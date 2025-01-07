#include "utils.h"

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
    for (int i = bdd_varnum() - 1; i >= 0; i--)
        if (vprof[i] > 0)
            (*indexes)[--count] = i;

    if (vprof) free(vprof);    // spot does not free this mem.
}

std::set<int> operator+(const std::set<int> &s1, const std::set<int> &s2) {
  std::set<int> res = {};
  std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(res, res.end()));
  return res;
}

std::set<int> operator-(const std::set<int> &s1, const std::set<int> &s2) {
  std::set<int> res = {};
  std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(res, res.end()));
  return res;
}

bool contains(const std::set<int> *set, const int value) {
    return set->find(value) != set->end();
}

std::ostream &operator<<(std::ostream &os, const std::set<int> &s) {
  std::set<int>::iterator it;
  os << "{";
  for (it = s.begin(); it != s.end(); it++) {
    if (it != s.begin())
      os << ", ";
    os << *it;
  }
  os << "}";

  return os;
}
