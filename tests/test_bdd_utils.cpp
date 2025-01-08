#include "utils.h"
#include <spot/parseaut/public.hh>

void test_bdd_var_indexes(const bdd &expr, const int *expect_indexes, const unsigned int expect_size) {
    int *indexes = nullptr;
    unsigned int size = UINT_MAX;

    /* Explicitly set size > 0, to check that it is correctly changed. */
    assert(size > 0);

    hoax::bdd_var_indexes(expr, &indexes, &size);

    /* Either pointer may only be nullptr if both are! */
    assert((indexes == nullptr) == (expect_indexes == nullptr));

    /* The bdd contains no vars, so the size must be set to 0! */
    assert(size == expect_size);

    if (indexes && expect_indexes)
      for (unsigned int i = 0; i < size; i++)
        /* The index arrays must must match. */
        assert(indexes[i] == expect_indexes[i]);

    if (indexes)
      free(indexes);
}

int test_bdd_utils(const spot::twa_graph_ptr aut) {
  assert(aut != nullptr);

  /* Require an exact variable count for the tests.
    Spot bdds do not throw many errors, so this is necessary. */
  assert(bdd_varnum() == 3);

  bdd a, b, c, expr, all;
  a = bdd_ithvar(0);
  b = bdd_ithvar(1);
  c = bdd_ithvar(2);
  all = aut->ap_vars();

  {
    /* Test the default return value: always return bddtrue if the expression
      contains no variables. */
    expr = bddtrue;
    assert(hoax::bdd_variables(expr) == bddtrue);
    expr = bddfalse;
    assert(hoax::bdd_variables(expr) == bddtrue);

    /* If all variables are used, then all of them are returned.
      These test cases makes the contrast with `hoax::bdd_variablescomp()` explicit. */
    expr = all;
    assert(hoax::bdd_variables(expr) == all);

    /* Find variables for a conjunction with no negations. */
    expr = a & b;
    assert(hoax::bdd_variables(expr) == (a & b));

    /* Find variables for a conjunction with negations. */
    expr = a & !c;
    assert(hoax::bdd_variables(expr) == (a & c));

    /* Find variables for a complex expression. */
    expr = (a & !c) | (b & c & a);
    assert(hoax::bdd_variables(expr) == (a & b & c));
  }

  {
    /* Test the default return value: always return bddtrue if the expression
      contains all variables. */
    expr = all;
    assert(hoax::bdd_variablescomp(expr) == bddtrue);

    /* If no are used, return a conjunction of all variables.
      These test cases makes the contrast with `hoax::bdd_variables()` explicit. */
    expr = bddtrue;
    assert(hoax::bdd_variablescomp(expr) == all);
    expr = bddfalse;
    assert(hoax::bdd_variablescomp(expr) == all);

    /* Find variables for a conjunction with no negations. */
    expr = a & b;
    assert(hoax::bdd_variablescomp(expr) == c);

    /* Find variables for a conjunction with negations. */
    expr = a & !c;
    assert(hoax::bdd_variablescomp(expr) == b);

    /* Find variables for a complex expression. */
    expr = (a & !c) | (b & c & a);
    assert(hoax::bdd_variablescomp(expr) == bddtrue);
  }

  {
    /* Test the base cases, where no indexes are present. */
    test_bdd_var_indexes(bddtrue, nullptr, 0);
    test_bdd_var_indexes(bddfalse, nullptr, 0);
  }

  {
    /* Test an edge case, where all indexes are present. */
    expr = all;
    int indexes[] = {0, 1, 2};
    unsigned int size = 3;
    test_bdd_var_indexes(expr, indexes, size);
  }

  {
    /* Test a simple expression without negations. */
    expr = a & b;
    int indexes[] = {0, 1};
    unsigned int size = 2;
    test_bdd_var_indexes(expr, indexes, size);
  }

  {
    /* Test a simple expression with negations. */
    expr = (!a) & c;
    int indexes[] = {0, 2};
    unsigned int size = 2;
    test_bdd_var_indexes(expr, indexes, size);
  }

  {
    /* Test a complex expression. */
    expr = (a & !c) | (b & c & a);
    int indexes[] = {0, 1, 2};
    unsigned int size = 3;
    test_bdd_var_indexes(expr, indexes, size);
  }

  return 0;
}

int main(int argc, char *argv[])
{
  assert(argc >= 2);  // Require the input path.
  std::string dir_in  = argv[1];
  int ret = 0;

  /* Parse an automaton to initialize `bdd_varnum()`. */
  std::string path_in = dir_in + "/hoa_benchmarks/toy_example_1.ehoa";
  auto pa = spot::parse_aut(path_in, spot::make_bdd_dict());

  ret = test_bdd_utils(pa->aut);
  if (ret) return ret;

  return 0;
}