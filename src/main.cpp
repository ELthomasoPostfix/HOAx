#include "hoax.h"
#include <iostream>
#include <getopt.h>

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
    printf ("parity: %s\n", argv[optind++]);
    // The eHOA format adds a 'controllable-AP' input property.
    // Spot should retain this in the form of the named prop
    // 'synthesis-outputs' in each spot::twa_graph after simply
    // calling spot::parse_aut. See the README for links and
    // details?
    zielonka();
  }


  return 0;
}