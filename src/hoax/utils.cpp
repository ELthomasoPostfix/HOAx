#include "utils.h"

bdd hoax::bdd_variables(const bdd &r) {
    return hoax::bdd_variables_(r, true);
}

bdd hoax::bdd_variablescomp(const bdd &r) {
    return hoax::bdd_variables_(r, false);
}

bdd hoax::bdd_variables_(const bdd &r, const bool var_should_appear) {
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

void hoax::bdd_var_indexes(const bdd &r, int **indexes, unsigned int *size) {
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

std::set<int> hoax::operator+(const std::set<int> &s1, const std::set<int> &s2) {
  std::set<int> res = {};
  std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(res, res.end()));
  return res;
}

std::set<int> hoax::operator-(const std::set<int> &s1, const std::set<int> &s2) {
  std::set<int> res = {};
  std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(res, res.end()));
  return res;
}

bool hoax::contains(const std::set<int> *set, const int value) {
    return set->find(value) != set->end();
}

std::ostream &hoax::operator<<(std::ostream &os, const std::set<int> &s) {
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

void hoax::to_dot(const std::filesystem::path &path_in, const std::filesystem::path &path_out,
            const spot::twa_graph_ptr aut) {
    std::string path_out_dot = path_out.string() + path_in.filename().string() + ".dot";
    std::ofstream dot_of(path_out_dot);
    if (!dot_of.is_open())
        throw std::runtime_error("Could not open ofstream to print to dot.");
    spot::print_dot(dot_of, aut);
}
