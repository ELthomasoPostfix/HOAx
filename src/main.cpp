#include "hoax.h"
#include "utils.h"
#include <iostream>
#include <getopt.h>
#include <fstream>
#include <filesystem>
#include <spot/parseaut/public.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twaalgos/game.hh>

/* Flag set by "--verbose" */
static int flag_verbose = 0;

int main(int argc, char *argv[]) {
  while (true) {
    switch (getopt(argc, argv, "hv")) {
      case 'v':
        flag_verbose = 1;
        continue;

      case 'h': {
        std::cout << "Usage: hoax [options] [arguments]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h             Show this help message and exit" << std::endl;
        std::cout << "  -v             Enable verbose output" << std::endl;
        std::cout << "Arguments:" << std::endl;
        std::cout << "  A space separated list of file paths to" << std::endl;
        std::cout << "  eHOA input files of parity games (arenas)" << std::endl;
        exit(0);
      }
    }
    /* End of options. */
    break;
  }

  if (flag_verbose)
    puts ("verbose flag is set");

  while (optind < argc) {
    char *path_in = argv[optind++];


    // Customize parser options. See
    // https://spot.lre.epita.fr/doxygen/structspot_1_1automaton__parser__options.html
    spot::automaton_parser_options opts = {true, false, true, false, false, true};
    spot::environment& env = spot::default_environment::instance();
    // Parse a HOA file. See
    // https://spot.lre.epita.fr/doxygen/group__twa__io.html#ga7ddd70d2b02e1234814a2f7fa6afe052
    spot::parsed_aut_ptr pa = spot::parse_aut(path_in, spot::make_bdd_dict(), env, opts);
    if (flag_verbose && pa->format_errors(std::cout)) {
      printf("SKIP\tFORMAT ERR\t%s\n", path_in);
      continue;
    }
    if (pa->aborted) {
      printf("SKIP\tABORT\t%s\n", path_in);
      continue;
    }

    spot::twa_graph_ptr aut = pa->aut;
    if (aut == nullptr) {
      printf("SKIP\tTwA MISSING\t%s\n", path_in);
      continue;
    }

    std::string prop_name;
    // The "state-player" prop is NOT specified in the benchmarks!
    // Since it's a spot extension to HOA, our inputs should not have it!
    prop_name = "state-player";
    auto state_player = aut->get_named_prop<std::vector<bool>>(prop_name);
    if (state_player != nullptr) {
      printf("SKIP\tPROP UNEXPECTED %s\t%s\n", prop_name.c_str(), path_in);
      continue;
    }

    // The "state-winner" prop is probably only set by spot after solving
    // the parity game with its builtin algo?
    // Since it's a spot extension to HOA, our inputs should not have it!
    prop_name = "state-winner";
    auto state_winner = aut->get_named_prop<std::vector<bool>>(prop_name);
    if (state_winner != nullptr) {
      printf("SKIP\tPROP UNEXPECTED %s\t%s\n", prop_name.c_str(), path_in);
      continue;
    }

    // The "strategy" prop is probably only set by spot after solving
    // the parity game with its builtin algo?
    // Since it's a spot extension to HOA, our inputs should not have it!
    prop_name = "strategy";
    auto strategy = aut->get_named_prop<std::vector<unsigned>>(prop_name);
    if (strategy != nullptr) {
      printf("SKIP\tPROP UNEXPECTED %s\t%s\n", prop_name.c_str(), path_in);
      continue;
    }

    /* The "synthesis-outputs" prop is set by the "Controllable AP" eHOA header.
      The type of this named prop is `bdd`, NOT `spot::bdd_dict`!
      That typing mistake cost me hours of debugging ... curse you, inheritance!

      For an example of using the "synthesis-outputs" bdd, see spot source code:
          https://gitlab.lre.epita.fr/spot/spot/-/blob/next/bin/ltlsynt.cc#L903

      The "synthesis-outputs" bdd is a confunction of all controllable APs.
      i.e.

          AP: 4 "a" "b" "c" "d"
          controllable-AP: 0 2

      results in a "synthesis-outputs" bdd that represents the propositional
      formula (a & c).
     */
    prop_name = "synthesis-outputs";
    auto synth_out = aut->get_named_prop<bdd>(prop_name);
    if (synth_out == nullptr) {
      printf("SKIP\tPROP MISSING %s\t%s\n", prop_name.c_str(), path_in);
      continue;
    }

    /* Enforce the automaton types and properties we expect in the benchmarks.
     */
    bool pmax = true;  // default = max
    bool podd = false; // default = even
    if (!(aut->acc().is_parity(pmax, podd) || aut->acc().is_f())) {
      printf("WARN\tNON-PARITY TwA? '%s'\t%s\n", aut->acc().name().c_str(), path_in);
      continue;
    }

    /* Given the following example HOA headers

          acc-name: parity max even X
          Acceptance: X ...

      each of the X acceptance sets corresponds to a priority level of the
      parity game arena's priority function p, as specified in zielonka.
      So, the priority levels are 0, 1, ..., X.

      Note that every transition in the automaton should be part of EXACTLY one
      acceptance set, else the transition would have more than one possible
      priority level!
    */
    bool found_invalid_edge = false;
    for (const auto &edge : aut->edges()) {
      if (!edge.acc.is_singleton()) {
        found_invalid_edge = true;
        printf("SKIP\t EDGE HAS %i ACCEPTING SETS, EXPECTED EXACTLY 1\t%s\n", edge.acc.count(), path_in);
        break;
      }
    }
    if (found_invalid_edge) {
      continue;
    }


    // TODO: DELETE \/ DELETE \/ DELETE \/ DELETE \/
    std::string path_out_dot = "output/" + std::filesystem::path(path_in).filename().string() + ".dot";
    std::ofstream dot_of(path_out_dot);
    if (!dot_of.is_open()) return 1;
    spot::print_dot(dot_of, aut);
    std::cout << "Generated dot file at: " << path_out_dot << std::endl;
    // TODO: DELETE /\ DELETE /\ DELETE /\ DELETE /\ 



    // TODO: Expand each edge formula into separate transitions with as
    //       label one of the valuations of that formula?
    /* Note: Spot makes the distinction between transitions (labeled with
       a single AP) and edges (labeled with an entire propositional
       formula):
          https://spot.lre.epita.fr/concepts.html#trans-edge
       So, use `new_edge(...)` instead of `new_transition(...)`?
    */



    // TODO: For Buchi automata, annotate any transition without acceptance set
    //       with odd parity/acceptance set 1? Because we want to solve for
    //       parity even, so all transitions/states already annotated correspond
    //       with even/0 parity? Or just re-read the mail of Guillermo.



    /* See the spot parser for a practical example of constructing a bdd
      for given variable valuations:
          https://gitlab.lre.epita.fr/spot/spot/-/blob/next/spot/parseaut/parseaut.yy#L441
    */

    /* TODO: Move these references to the func docs of zielonka() ????

      See spot's `bdd` operation implementations `bdd_exist()`, `quantify(...)`
      `bdd_existcomp()`, ... in the following section of spot's docs:
          https://gitlab.lre.epita.fr/spot/spot/-/blob/next/buddy/src/bddop.c#L2212
     */



    // std::cout << "heyo: " << *synth_out << std::endl;
    // std::cout << "heyo: " << std::min(1, 2) << std::endl;

    // std::set<HOAxState> test_set_h;
    // test_set_h.insert(std::make_pair(1, bddfalse));
    // test_set_h.insert(std::make_pair(1, bddtrue));
    // test_set_h.insert(std::make_pair(2, bddtrue));
    // test_set_h.insert(std::make_pair(2, bddfalse));

    // std::cout << (bddtrue < bddfalse) << std::endl;

    // std::cout << "heyo: ";
    // for (auto &e : test_set_h)
    //   std::cout << e << ", ";
    // std::cout << std::endl;



    // std::set<std::pair<int, int>> test_set;
    // test_set.insert(std::make_pair(1, 1));
    // test_set.insert(std::make_pair(1, 2));
    // test_set.insert(std::make_pair(2, 1));
    // test_set.insert(std::make_pair(2, 2));
    // std::cout << "heyo: ";
    // for (auto &e : test_set)
    //   std::cout << "{" << e.first << ", " << e.second << "}" << ", ";
    // std::cout << std::endl;



    std::set<int> W1 = {};
    std::set<int> W2 = {};
    std::set<int> vertices = {};
    std::set<int> vertices_even = {};

    zielonka(&W1, &W2, &vertices, &vertices_even, aut, pmax);

    // The initial/start state.
    unsigned int sI = aut->get_init_state_number();

    // Call my own implementation of a parity game solver.
    const bool SOL_COMPUTED = podd ? contains(&W2, sI) : contains(&W1, sI);
    const std::string SOL_STR_COMPUTED = SOL_COMPUTED ? "REAL" : "UNREAL";

    // Compare against spot's implementation as a baseline.
    const bool SOL_ACTUAL = false;
    // const bool SOL_ACTUAL = spot::solve_parity_game(aut);  // TODO: Is this possible with our given benchmarks???
    const std::string SOL_STR_ACTUAL = SOL_ACTUAL ? "REAL" : "UNREAL";

    printf ("computed=%s actual=%s\t%s\n",
      SOL_STR_COMPUTED.c_str(), SOL_STR_ACTUAL.c_str(), path_in);
  }


  return 0;
}