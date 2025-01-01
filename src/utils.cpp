#include "utils.h"

bool contains(const std::set<int> *set, const int value) {
    return set->find(value) != set->end();
}
