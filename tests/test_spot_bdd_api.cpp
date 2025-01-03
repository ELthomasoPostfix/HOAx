#include <spot/twa/twagraph.hh>
#include <spot/parseaut/public.hh>
#include <spot/twaalgos/game.hh>
#include <stdio.h>

/** Some tests to verify my understanding of the bddx c++ interface.
 *
 * This test case works out basic usage of the bddx c++ interface for
 * bdd manipulation. This serves as documentation and a reference point
 * for the implementation of zielonka that follows later.
 */
int test_vars_to_bdd(const std::string &dir_in, const std::string &dir_out) {
  std::string path_in = dir_in + "/hoa_benchmarks/toy_example_1.ehoa";

  spot::parsed_aut_ptr pa = spot::parse_aut(path_in, spot::make_bdd_dict());
  const bdd synth_out = spot::get_synthesis_outputs(pa->aut);



  /* See the file bddx.h for the BDD interface, e.g. in the spot source code:
      https://gitlab.lre.epita.fr/spot/spot/-/blob/next/buddy/src/bddx.h#L514

     For the function `bdd_have_common_assignment` see
      https://gitlab.lre.epita.fr/spot/spot/-/blob/next/buddy/src/bddop.c#L3836
   */



  // Suppose the input file specifies AP = {a, b, c} = {0, 1, 2}.
  bdd bdd0 = bdd_ithvar(0);  // bdd = <a=1> = prop. formula "a"
  bdd bdd1 = bdd_ithvar(1);  // bdd = <b=1> = prop. formula "b"
  bdd bdd2 = bdd_ithvar(2);  // bdd = <c=1> = prop. formula "c"

  // The number of vars/APs in the bdds.
  int nr_vars = bdd_varnum();
  // We assume a specific number of APs in the following tests, so ensure that
  // assumption just to be sure.
  assert(nr_vars == 3);



  /* The function `bdd_forall(bdd& r, bdd& var)` removes all variables
    in var from r, see spot source code docs:
        https://gitlab.lre.epita.fr/spot/spot/-/blob/next/buddy/src/bddop.c#L2301

    Note that BDDs really encode boolean functions, and a different
    representation of boolean functions are propositional formulas.
    So we can express BDDs as propositional formulas.
   */
  // bdd_forall(<a=1>, <a=0>)
  // = bdd_forall(a, !a) == <> == false
  assert(bdd_forall(bdd0, !bdd0) == bddfalse);

  // bdd_forall(<a=1>, <a=1>)
  // = bdd_forall(a, a) == <> == false
  assert(bdd_forall(bdd0, bdd0) == bddfalse);

  // bdd_forall(<a=1>, <a=1, b=1>)
  // = bdd_forall(a, a & b) == <> == false
  assert(bdd_forall(bdd0, bdd0 & bdd1) == bddfalse);

  // bdd_forall(<a=1, b=1>, <a=1>)
  // = bdd_forall(a & (c | b), a)
  // = bdd_forall((a & c) | (a & b), c) == a & b
  assert(bdd_forall((bdd0 & (bdd2 | bdd1)), bdd2) == (bdd0 & bdd1));

  // There is no intersection between bdd <var1=1> and bdd <var0=1>.
  // Removing all variables part of <var1=1> from <var0=1> does not
  // change <var0=1>.
  assert(bdd_forall(bdd0, synth_out) == bdd0);

  // The bdd <var1=1> is equal to <var1=1>.
  // Said differently, Removing all variables part of <var1=1>
  // from <var1=1> results in no variables remaining; the result
  // can only ever resolve to false!
  assert(synth_out == bdd1 && bdd_forall(bdd1, synth_out) == bddfalse);

  // There is no intersection between bdd <var1=1> and bdd <var2=1>
  // Removing all variables part of <var1=1> from <var2=1> does not
  // change <var2=1>.
  assert(bdd_forall(bdd2, synth_out) == bdd2);



  /* Test functions related to restricting some variables in a bdd
    to constant true/false values:
      `bdd_restrict()`

    This corresponds to fixing a subset of variables in some propositional
    formula to true/false. Spot will simplify the given bdd to reflect the
    changes. e.g.
        a=true    a & b = true & b = b
        a=false   a & b = false & b = false
        a=true    (a | b) & c = (true | b) & c = true & c = c
        ...

    See the spot source code docs:
        https://gitlab.lre.epita.fr/spot/spot/-/blob/next/buddy/src/bddop.c#L1464
  */
  // a=true     a & b = true & b = b
  // Represent "a=true" by the prop. formula "a".
  assert(bdd_restrict(bdd0 & bdd1, bdd0) == bdd1);

  // a=false    a & b = false & b = false
  // Represent "a=false" by the prop. formula "!a".
  assert(bdd_restrict(bdd0 & bdd1, !bdd0) == bddfalse);

  // b=true     a | b = a | true = true
  // Represent "b=true" by the prop. formula "b".
  assert(bdd_restrict(bdd0 | bdd1, bdd1) == bddtrue);

  // b=false    a | b = a | false = a
  // Represent "b=false" by the prop. formula "!b".
  assert(bdd_restrict(bdd0 | bdd1, !bdd1) == bdd0);



  /* Test functions related to checking satisfiability of bdds:
      `bdd_satone()`, `bdd_fullsatone()`, `bdd_satcount()`,
      `bdd_satcountset()`, `bdd_allsat()`

    See the spot source code docs:
        https://gitlab.lre.epita.fr/spot/spot/-/blob/next/buddy/src/bddop.c#L2871

    They can be used to see if a restricted bdd can at all be satisfied.

    `bdd_satone()` just finds some assignment of a subset of all variables
     which satisfies the prop. formula represented by the bdd. i.e. the
     variables that do not appear in this formula, but are perhaps used
     in some other formulas, do not have to appear in the valuation/output.
     e.g. for APs = {a, b, c}
        bdd_satone(a & b) = a & b

    `bdd_fullsatone()` just finds some assignment of ALL variables
     which satisfies the prop. formula represented by the bdd. i.e. the
     variables that do not appear in this formula, but are perhaps used
     in some other formulas, must also have an assigned value in the
     valuation/output.
     e.g. for APs = {a, b, c}
        bdd_fullsatone(a & b) = a & b & !c
  */
  // a & b    is satisfied by    a=true, b=true
  bdd TT = bdd0 & bdd1; // a=true, b=true
  assert(bdd_satone(bdd0 & bdd1) == TT);

  // a | (!a & b)    can be satisfied by:
  //    a=true, b=don't care    OR
  //    a=false, b=true
  bdd FT = !bdd0 & bdd1;  // a=false, b=true
  assert(bdd_satone((bdd0 | (!bdd0 & bdd1))) == FT);

  // a & b    is satisfied by    a=true, b=true, c=don't care
  // ==> `bdd_fullsatone()` requires all variables to have a valuation.
  bdd TTF = bdd0 & bdd1 & !bdd2; // a=true, b=true, c=false
  assert(bdd_fullsatone(bdd0 & bdd1) == TTF);

  // a | (!a & b)    can be satisfied by:
  //    a=true, b=don't care, c=don't care    OR
  //    a=false, b=true, c=don't care
  // ==> `bdd_fullsatone()` requires all variables to have a valuation.
  bdd FTF = !bdd0 & bdd1 & !bdd2;  // a=false, b=true, c=false
  assert(bdd_fullsatone((bdd0 | (!bdd0 & bdd1))) == FTF);

  return 0;
}

int main(int argc, char *argv[])
{
  assert(argc == 3);  // Require the input & output paths.
  std::string dir_in  = argv[1];
  std::string dir_out = argv[2];
  int ret = 0;

  ret = test_vars_to_bdd(dir_in, dir_out);
  if (ret) return ret;

  return 0;
}