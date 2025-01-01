#include "hoax.h"
#include <iostream>
#include <getopt.h>
#include <spot/parseaut/public.hh>

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
    prop_name = "state-player";
    auto state_player = aut->get_named_prop<std::vector<bool>>(prop_name);
    if (state_player != nullptr) {
      printf("SKIP\tPROP UNEXPECTED %s\t%s\n", prop_name.c_str(), path_in);
      continue;
    }

    // The "state-winner" prop is probably only set after solving the parity game?
    prop_name = "state-winner";
    auto state_winner = aut->get_named_prop<std::vector<bool>>(prop_name);
    if (state_winner != nullptr) {
      printf("SKIP\tPROP UNEXPECTED %s\t%s\n", prop_name.c_str(), path_in);
      continue;
    }

    // The "strategy" prop is probably only set after solving the parity game?
    prop_name = "strategy";
    auto strategy = aut->get_named_prop<std::vector<unsigned>>(prop_name);
    if (strategy != nullptr) {
      printf("SKIP\tPROP UNEXPECTED %s\t%s\n", prop_name.c_str(), path_in);
      continue;
    }

    // The "synthesis-outputs" prop is set by the "Controllable AP" eHOA header.
    prop_name = "synthesis-outputs";
    auto synth_out = aut->get_named_prop<spot::bdd_dict>(prop_name);
    if (synth_out == nullptr) {
      printf("SKIP\tPROP MISSING %s\t%s\n", prop_name.c_str(), path_in);
      continue;
    }

    std::set<int> W1 = {};
    std::set<int> W2 = {};
    std::vector<int> vertices = {};
    std::vector<int> vertices_even = {};
    std::vector<int> edges = {};
    auto priority_fun = [](int vertex) -> int {
      return vertex;
    };

    zielonka(&W1, &W2, &vertices, &vertices_even, &edges, priority_fun);
    if (false)
      printf ("REAL\t%s\n", path_in);
    else
      printf ("UNREAL\t%s\n", path_in);
  }


  return 0;
}