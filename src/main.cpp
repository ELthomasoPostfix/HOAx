#include "hoax.h"
#include <iostream>

// TODO: Add a "--dry-run" flag, so that a user can verify that
// the "meson compile" command succeeded? Also add a mention in the
// Compile section of the README.
int main() {

  zielonka();

  // The eHOA format adds a 'controllable-AP' input property.
  // Spot should retain this in the form of the named prop
  // 'synthesis-outputs' in each spot::twa_graph after simply
  // calling spot::parse_aut. See the README for links and
  // details?
  std::cout << "Hello Main!" << std::endl;
  return 0;
}