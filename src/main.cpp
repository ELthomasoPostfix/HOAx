#include "hoax.h"
#include "utils.h"
#include <filesystem>
#include <iostream>
#include <getopt.h>
#include <spot/parseaut/public.hh>

/** Flag set by "-v" */
static int flag_verbose = 0;

/** Flag set by "-b" */
static int flag_baseline = 0;

/** The default output directory. */
const std::filesystem::path DEFAULT_DIR_OUT("output/");

/** The default input directory. */
const std::filesystem::path DEFAULT_DIR_IN("input/");

/** The maximum runtime allowed for solving a parity game, in seconds.
    Quit early if this deadline is exceeded. */
const unsigned int RUNTIME_MAX_SEC = 300;


int main(int argc, char *argv[]) {
  while (true) {
    switch (getopt(argc, argv, "hvb")) {
      case 'v':
        flag_verbose = 1;
        continue;

      case 'b':
        flag_baseline = 1;
        continue;

      case 'h': {
        std::cout << "Usage: hoax [options] [arguments]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h             Show this help message and exit" << std::endl;
        std::cout << "  -v             Enable verbose output" << std::endl;
        std::cout << "  -b             Call spot's parity game solver as a baseline comparison" << std::endl;
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
    /* Each game has an individual runtime deadline. */
    const clock_t start = clock();
    const clock_t deadline = start + RUNTIME_MAX_SEC * CLOCKS_PER_SEC;

    std::filesystem::path path_in(argv[optind++]);

    /* Ensure the input path exists. */
    if (!std::filesystem::exists(path_in)) {
      printf("SKIP\tPATH DOES NOT EXIST\t%s\n", path_in.c_str());
      continue;
    }

    // Customize parser options. See
    // https://spot.lre.epita.fr/doxygen/structspot_1_1automaton__parser__options.html
    spot::automaton_parser_options opts = {true, false, true, false, false, true};
    spot::environment& env = spot::default_environment::instance();
    // Parse a HOA file. See
    // https://spot.lre.epita.fr/doxygen/group__twa__io.html#ga7ddd70d2b02e1234814a2f7fa6afe052
    spot::parsed_aut_ptr pa = spot::parse_aut(path_in, spot::make_bdd_dict(), env, opts);
    if (flag_verbose && pa->format_errors(std::cout)) {
      printf("SKIP\tFORMAT ERR\t%s\n", path_in.c_str());
      continue;
    }
    if (pa->aborted) {
      printf("SKIP\tABORT\t%s\n", path_in.c_str());
      continue;
    }

    spot::twa_graph_ptr aut = pa->aut;
    if (aut == nullptr) {
      printf("SKIP\tTwA MISSING\t%s\n", path_in.c_str());
      continue;
    }

    std::string prop_name;
    // The "state-player" prop is NOT specified in the benchmarks!
    // Since it's a spot extension to HOA, our inputs should not have it!
    prop_name = PROP_SPOT_STATE_PLAYER;
    auto state_player = aut->get_named_prop<std::vector<bool>>(prop_name);
    if (state_player != nullptr) {
      printf("SKIP\tPROP UNEXPECTED %s\t%s\n", prop_name.c_str(), path_in.c_str());
      continue;
    }

    // The "state-winner" prop is probably only set by spot after solving
    // the parity game with its builtin algo?
    // Since it's a spot extension to HOA, our inputs should not have it!
    prop_name = PROP_SPOT_STATE_WINNER;
    auto state_winner = aut->get_named_prop<std::vector<bool>>(prop_name);
    if (state_winner != nullptr) {
      printf("SKIP\tPROP UNEXPECTED %s\t%s\n", prop_name.c_str(), path_in.c_str());
      continue;
    }

    // The "strategy" prop is probably only set by spot after solving
    // the parity game with its builtin algo?
    // Since it's a spot extension to HOA, our inputs should not have it!
    prop_name = PROP_SPOT_STRAT;
    auto strategy = aut->get_named_prop<std::vector<unsigned>>(prop_name);
    if (strategy != nullptr) {
      printf("SKIP\tPROP UNEXPECTED %s\t%s\n", prop_name.c_str(), path_in.c_str());
      continue;
    }

    /* The "synthesis-outputs" prop is set by the "controllable-AP" eHOA header.
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
    prop_name = PROP_SPOT_SYNTH_OUTPUT;
    auto synth_out = aut->get_named_prop<bdd>(prop_name);
    if (synth_out == nullptr) {
      printf("SKIP\tPROP MISSING %s\t%s\n", prop_name.c_str(), path_in.c_str());
      continue;
    }

    /* Enforce the automaton types and properties we expect in the benchmarks.
     */
    bool pmax = true;  // default = max
    bool podd = false; // default = even
    if (!(aut->acc().is_parity(pmax, podd, false) || aut->acc().is_f() || aut->acc().is_t())) {
      printf("WARN\tNON-PARITY TwA? '%s'\t%s\n", aut->acc().name().c_str(), path_in.c_str());
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
        printf("SKIP\t EDGE HAS %i ACCEPTING SETS, EXPECTED EXACTLY 1\t%s\n", edge.acc.count(), path_in.c_str());
        break;
      }
    }
    if (found_invalid_edge)
      continue;

    // TODO: For Buchi automata, annotate any transition without acceptance set
    //       with odd parity/acceptance set 1? Because we want to solve for
    //       parity even, so all transitions/states already annotated correspond
    //       with even/0 parity? Or just re-read the mail of Guillermo.


    try {
      hoax::HOAxParityTwA hptwa = hoax::HOAxParityTwA(aut, start, deadline);
      hptwa.set_state_names();

      // Call my own implementation of a parity game solver.
      // If the player we expect to win equals the player that actually wins,
      // then the game is realizable.
      const bool SOL_COMPUTED = podd == hptwa.solve_parity_game(pmax);
      const std::string SOL_STR_COMPUTED = SOL_COMPUTED ? "REAL" : "UNREAL";

      if (flag_verbose) {
        // hoax::to_dot(path_in, DEFAULT_DIR_OUT, hptwa.src);
        // hoax::to_dot(path_in, DEFAULT_DIR_OUT, hptwa.exp);
      }

      if (flag_baseline) {
        /* Compare against spot's implementation as a baseline.

          Spot's docs state about the arena input that "The arena is a
          deterministic max odd parity automaton with a 'state-player'
          property." See
              https://spot.lre.epita.fr/doxygen/group__games.html#ga5282822f1079cdefc43a1d1b0c83a024
          However, if an automaton's acceptance condition is not "parity max odd",
          then the source code for `spot::solve_parity_game()` actually internally
          transforms the automaton to "parity max even", and solving for "parity max even"
          in the transformed automaton is equivalent to solving the original acceptance
          condition in the original/untransformed automaton.
          That is to say, spot's `spot::solve_parity_game()` outputs which player wins
          in the initial state (false if even wins, true if odd wins), regardless of the
          acceptance condition on the original automaton, and without intervention of
          the caller.
            https://gitlab.lre.epita.fr/spot/spot/-/blob/next/spot/twaalgos/game.cc#L312
        */
        const bool SOL_ACTUAL = podd == spot::solve_parity_game(hptwa.exp);
        const std::string SOL_STR_ACTUAL = SOL_ACTUAL ? "REAL" : "UNREAL";

        if (flag_verbose) {
          auto state_winners_spot = hptwa.exp->get_or_set_named_prop<std::vector<bool>>(PROP_SPOT_STATE_WINNER);
          auto state_winners_hoax = hptwa.exp->get_or_set_named_prop<std::vector<bool>>(PROP_HOAX_STATE_WINNER);
          const bool same_winners = (*state_winners_spot == *state_winners_hoax);
          std::string diagnostic = "";

          if (!same_winners) {
            unsigned int nr_diffs = 0;
            for (unsigned int i = 0; i < state_winners_spot->size(); i++)
              nr_diffs += (state_winners_spot->at(i) != state_winners_hoax->at(i));
            diagnostic += "#differences / #states = " + std::to_string(nr_diffs) +
              " / " + std::to_string(state_winners_spot->size());
            if (nr_diffs == state_winners_spot->size())
              diagnostic += " (Spot and HOAx have inverted winners!)";
          }

          std::cout << "Winners(spot) == Winners(hoax) : " << same_winners << "\t" << diagnostic << std::endl;
        }

        const std::string sodd = podd ? "ODD" : "EVEN";
        const std::string smax = pmax ? "MAX" : "MIN";

        printf ("<%s, %s> %s %s\t%s\t%s\n",
          smax.c_str(), sodd.c_str(), SOL_STR_COMPUTED.c_str(),
          SOL_STR_ACTUAL.c_str(), (std::to_string((clock() - hptwa.start) / (float)CLOCKS_PER_SEC) + "s").c_str(), path_in.c_str());
      }
      else
        printf ("%s\t%s\n",
          SOL_STR_COMPUTED.c_str(), path_in.c_str());
    } catch (std::runtime_error &e) {
      std::cout << "SKIP\t" << e.what() << "\t" << path_in.string() << std::endl;
    }
  }


  return 0;
}