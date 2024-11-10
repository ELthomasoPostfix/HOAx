#include <spot/tl/parse.hh>
#include <spot/tl/formula.hh>
#include <spot/twa/bdddict.hh>
#include <spot/twa/twagraph.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twaalgos/game.hh>
#include <spot/parseaut/public.hh>
#include <spot/twaalgos/translate.hh>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

int test_ltl(const std::string &dir_in, const std::string &dir_out) {
  /* Test parsing an LTL formula into a standard HOA format automaton
    dot file output. */

  std::string path_in = dir_in + "/test_spot_parser_ltl.txt";
  std::string path_out = dir_out + "/test_spot_parser_ltl.dot";

  // Test that the Spot parser API provides the expected function signatures.
  std::ifstream ltl_if(path_in);
  if (!ltl_if.is_open()) return 1;

  std::string data;
  ltl_if >> data;
  // https://spot.lre.epita.fr/doxygen/group__tl__io.html#gadf461e56ae02af07eada0b081aca019d
  spot::formula ltl = spot::parse_formula(data);
  spot::translator tl = spot::translator();
  // https://spot.lre.epita.fr/doxygen/classspot_1_1translator.html#a36cdf71031732eddd084eb3abbbb5738
  spot::twa_graph_ptr twa_automaton = tl.run(&ltl);

  // Generate dot images.
  std::ofstream dot_of(path_out);
  if (!dot_of.is_open()) return 1;

  spot::print_dot(dot_of, twa_automaton);
  std::cout << "Generated dot file at: " << path_out << std::endl << std::endl;

  return 0;
}

int test_ehoa(const std::string &dir_in, const std::string &dir_out) {
  /* Test parsing an extended HOA (eHOA) format file into a standard HOA
    format automaton dot file output. */

  std::vector<std::string> paths_in = {
    "/hoa_benchmarks/Par_Onebit_(datasize=4_capacity=1_windowsize=1)eq=strong-bisim.gm.bz2.ehoa",
    "/hoa_benchmarks/StarNesterk=1_n=8_compact.gm.bz2.ehoa"
  };

  for(size_t idx = 0; idx < paths_in.size(); ++idx) {

    std::string path_in = dir_in + paths_in.at(idx);
    std::string path_out_dot = dir_out + "/test_spot_parser_ehoa" + std::to_string(idx) + ".dot";
    std::string path_out_ehoa = dir_out + "/test_spot_parser_ehoa" + std::to_string(idx) + ".ehoa";

    spot::bdd_dict_ptr bdd_dict = spot::make_bdd_dict();
    spot::parsed_aut_ptr aut = nullptr;
    spot::twa_graph_ptr twa_automaton = nullptr;

    // Parse a HOA file:
    //  https://spot.lre.epita.fr/doxygen/group__twa__io.html#ga7ddd70d2b02e1234814a2f7fa6afe052
    aut = spot::parse_aut(path_in, bdd_dict);
    twa_automaton = aut->aut;
    if (twa_automaton == nullptr) return 1;

    // The "controllable-AP" property is represented by the named property
    // "synthesis-outputs" defined in this table:
    //    https://spot.lre.epita.fr/concepts.html#named-properties
    // Related props are "state-winner", "strategy" and "synthesis-outputs".
    std::string prop_name = "synthesis-outputs";
    const spot::bdd_dict *synth_out;
    synth_out = aut->aut->get_named_prop<spot::bdd_dict>(prop_name);

    // If this prop is not present, then assume the "controllable-AP"
    // property was not parsed correctly!
    if (synth_out == nullptr)
      return 1;
    else
      std::cout << "The named prop \"synthesis-outputs\" is found in the TwA. "
                << "The \"controllable-AP\" prop was likely parsed properly."
                << std::endl;

    // Generate dot images.
    std::ofstream dot_of(path_out_dot);
    if (!dot_of.is_open()) return 1;
    std::ofstream ehoa_of(path_out_ehoa);
    if (!ehoa_of.is_open()) return 1;

    spot::print_dot(dot_of, twa_automaton);
    std::cout << "Generated dot file at: " << path_out_dot << std::endl;
    spot::print_hoa(ehoa_of, twa_automaton);
    std::cout << "Generated dot file at: " << path_out_ehoa << std::endl << std::endl;
  }

  return 0;
}

int main(int argc, char *argv[])
{

  assert(argc == 3);  // Require the input & output paths.
  std::string dir_in  = argv[1];
  std::string dir_out = argv[2];
  int ret = 0;

  ret = test_ltl(dir_in, dir_out);
  if (ret) return ret;

  ret = test_ehoa(dir_in, dir_out);
  if (ret) return ret;

  return 0;
}