#include "utils.h"
#include <iostream>

int test_set_utils() {

  std::set<int> s1 = {1,    3,    5};
  std::set<int> s2 = {1, 2,    4, 5};
  std::set<int> empty = {};

  {
    std::set<int> computed;
    std::set<int> expect;

    /* Test set union with one or more operands being empty. */
    computed = s1 + empty;
    expect = {1, 3, 5};
    assert(computed == expect);

    computed = empty + s2;
    expect = {1, 2, 4, 5};
    assert(computed == expect);

    computed = empty + empty;
    expect = {};
    assert(computed == expect);

    /* Test the general set union case. */
    computed = s1 + s2;
    expect = {1, 2, 3, 4, 5};
    assert(computed == expect);

    computed = s2 + s1;
    expect = {1, 2, 3, 4, 5};
    assert(computed == expect);
  }

  {
    std::set<int> computed;
    std::set<int> expect;

    /* Test set difference with one or more operands being empty. */
    computed = s1 - empty;
    expect = {1, 3, 5};
    assert(computed == expect);

    computed = empty - s2;
    expect = {};
    assert(computed == expect);

    computed = empty - empty;
    expect = {};
    assert(computed == expect);

    /* Test the general set difference case. */
    computed = s1 - s2;
    expect = {3};
    assert(computed == expect);

    computed = s2 - s1;
    expect = {2, 4};
    assert(computed == expect);
  }

  {
    /* Test set membership. */
    assert(!contains(&s1, 0));
    assert(contains(&s1, 1));
    assert(!contains(&empty, 0));
    assert(!contains(&empty, 1));
  }

  return 0;
}

int main()
{
  int ret = test_set_utils();
  if (ret) return ret;

  return 0;
}