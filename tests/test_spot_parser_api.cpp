#include <spot/tl/parse.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twaalgos/translate.hh>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

int main(int argc, char *argv[])
{

  assert(argc == 3);  // Require the input & output paths.
  std::string path_in  = argv[1];
  std::string path_out = argv[2];
  path_in += "/parser_test.txt";
  path_out += "/parser_test.dot";

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

  return 0;
}