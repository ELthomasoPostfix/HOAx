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
