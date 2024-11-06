#include <spot/tl/parse.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twaalgos/translate.hh>
#include <iostream>
#include <fstream>
#include <string>

int main()
{
  std::ifstream ltl_if("input/parser_test.txt");
  if (!ltl_if.is_open()) return 1;

  std::string data;
  ltl_if >> data;
  // https://spot.lre.epita.fr/doxygen/group__tl__io.html#gadf461e56ae02af07eada0b081aca019d
  spot::formula ltl = spot::parse_formula(data);
  spot::translator tl = spot::translator();
  // https://spot.lre.epita.fr/doxygen/classspot_1_1translator.html#a36cdf71031732eddd084eb3abbbb5738
  spot::twa_graph_ptr twa_automaton = tl.run(&ltl);

  // Generate dot images.
  std::ofstream dot_of("output/parser_test.dot");
  if (!dot_of.is_open()) return 1;

  spot::print_dot(dot_of, twa_automaton);

  return 0;
}