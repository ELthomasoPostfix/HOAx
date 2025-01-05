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

#endif
