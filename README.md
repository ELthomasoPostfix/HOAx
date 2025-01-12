# HOAx

A repo containing the project for the course Specification and Verification.

The goal is preparing a (eHOA) parity game solver for the [SYNTCOMP](https://www.syntcomp.org/) 2025 competition.

This project is a **hoax**, since despite claiming to implement a parity game solver, it heavily relies on external libraries (Spot) to achieve this. A true computer scientist would reinvent the wheel.

# Setup & Run - Basic

Utility scripts, `build.sh` and `run.sh`, are included at the root of the project for easier setup and running.

**Alternatively, see the github actions [workflow yml](/.github/workflows/meson-actions.yml).**

```sh
# Clone the repo
git clone https://github.com/ELthomasoPostfix/HOAx.git

# Install dependencies
sudo wget -q -O - https://www.lrde.epita.fr/repo/debian.gpg | sudo apt-key add -
echo 'deb http://www.lrde.epita.fr/repo/debian/ stable/' | sudo tee -a /etc/apt/sources.list
sudo apt-get update
sudo apt-get install libspot-dev meson

# Call the meson build pipeline.
./build.sh

# If the benchmarks are setup at /input/parity/parity/...
# then you can run all benchmarks via the utility script.
./run.sh

# Else manually run the entrypoint: pass a list of .ehoa file paths.
# For example:
./builddir/hoax input/hoa_benchmarks/diff-ACCs-*.ehoa

# Or call with the help prompt for a list of flags
./builddir/hoax -h
```

For more details, see [the detailed run section](#run).

# Implementation - Basic

A basic parity game solver consists of implementing zielonka's recursive algorithm. Note that spot has an [implementation](https://spot.lre.epita.fr/doxygen/group__games.html#ga5282822f1079cdefc43a1d1b0c83a024) of zielonka's algorithm as well, but, of course, we should implement the algorithm ourselves.

The core of zielonka is implemented in `hoax::zielonka` in [hoax.h](/src/hoax/hoax.h#L113), and the attractors in `hoax::attractors` in [hoax.h](/src/hoax/hoax.h#L140). However, the zielonka implementation expects the even player states to be explicit in the parity game arena. So an explicit translation step is required. This is why solving the parity game should be called through `hoax::HOAxParityTwA::solve_parity_game()`. This enforces the translation (also called an expansion) of the input automaton for use in zielonka. That function returns the player that wins the parity game according to the acceptance condition. So the caller must still check that the winner coincides with the _expected_ winner. For example, if Eve wins, but Adam was expected to win, then the game is _UNREALIZABLE_.

Note that spot always encodes acceptance sets on the transitions, i.e. state based acceptance is implicit while transition-based acceptance is explicit. But, zielonka by definition makes use of state-based priorities. To bridge the gap, we simply derive state-based priority from transition-based priority, taking into account the `min/max` acceptance condition.

We define the priority of a transition to be the `min/max` of the acceptance sets that that transition is a part of. The priority of a state is then the `min/max` of the priorities of all of its outgoing transitions. We can pre-compute all priorities before calling zielonka.

For more details, see [the implementation details](#implementation-details).


# Run

This section gives a more thorough explanation of how to run the project, and some of the implementation details. This may be relevant to the evaluation of the project w.r.t. the course.

**Alternatively, see the github actions [workflow yml](/.github/workflows/meson-actions.yml) for an ubuntu linux example of setting up and running the project.**

## Project - Setup

Before the project can be run, the dependencies must be setup. See the [dependencies section](#dependencies) on how to do that.

## Project - Part 1: Setup of the Parser

The first task for the project is setting up a parser that is able to read some input format and display it as a dot format image.
The project specifies several tests to verify this functionality. These tests are also run as part of the [meson pipeline](#compilation), when running `build.sh`.

[The linkage test](/tests/test_spot_linkage.cpp) verifies that Spot was installed correctly. If this script does not result in any compilation errors, and prints to the terminal, then all is well. The main concern is that you should explicitly link spot when compiling, as follows: `g++ ... -lspot -o ...`. The placement of `-lspot` in this command matters.

[The parser API test](/tests/test_spot_parser_api.cpp) shows that the Spot parser API exposes the expected functions. To provide visual indication of this, the test should also take some input file from the [input directory](/input/) and produce the dot image of the corresponding automaton into the [output directory](/output/). It also dumps the HOA of the automaton it parsed. This is to show that the `controllable-AP` property is preserved when an input automaton is specified in eHOA, meaning the input contains `controllable-AP`, since the output HOA file also specifies the original `controllable-AP` property value.

## Project - Part 2: Solve parity game realizability

Deciding **realizability** for a parity game means checking that player 0, Eve, has a winning strategy, which is equivalent with checking that player 1, Adam, does *not* have a winning strategy, or vice versa. This differs from **synthesis**, where you must not only decide realizability, but also output such a winning strategy (in AIGER format for SYNTCOMP).

### Run

To run the entrypoint for part two on the entire SYNTCOMP benchmark suite, run the following.

```sh
# The full benchmarks, if they are present at
#       input/parity/parity/keiren/*.ehoa
#       input/parity/parity/mucalc_mc/*.ehoa
#       input/parity/parity/pgsolver_based/*.ehoa
#       input/parity/parity/tlsf_based/*.ehoa
./run.sh

# Or a "brief" test of only the mucalc_mc subset of the benchmark
# that is part of this repository.
./run.sh --brief
```

### Implementation details

The basic implementation for solving parity game realizability is **Zielonka's algorithm**. This algorithm was described in the course document "playing games to synthesize reactive systems" in the section "A Divide-and-Conquer Algorithm for Parity Games". Pseudocode for the zielonka algorithm can also be found on the [wikipedia page](https://en.wikipedia.org/wiki/Parity_game#Recursive_algorithm_for_solving_parity_games) of parity games.

The algorithm is implemented in `hoax::zielonka`, but should actually be used through the public member function of the `HOAxParityTwA` class, `hoax::HOAxParityTwA::solve_parity_game()`, in [hoax.h](/src/hoax/hoax.h). This enforces the construction of a `HOAxParityTwA` object before zielonka can be called. This is crucial, since the zielonka implementation assumes the original parity automaton has been transformed.

### Automaton Transformation

The input `.ehoa` files specify parity games. Each game only contains the "odd player states", i.e. the states belonging to player 1 or player Adam. Each of the edges specifies a propositional formula that must be satisfied before that edge can be taken.

The "even player states", i.e. the state belonging to player 0 or player Eve, are implicit in the automaton. That is because the game is player as follow:
1. Player 1 chooses some evaluation of the uncontrollable APs.
2. Player 2 tries to choose some evaluation of the controllable APs so that a transition can still be taken.
3. If the min/max transition parity seen along the infinite path is even, then Eve wins, else Adam wins.

This means we can make the Eve states explicit in the automaton by performing a translation. For every Adam state $s_1$, for every evaluation of the uncontrollable APs $e_u$, we can add one Eve state $s_0$. Always add the edge $(s_1, s_0, e_u, w_\emptyset)$ which is the edge from the original Adam state to the new Eve state annotated with the uncontrollable AP evaluation $e_u$, and $w_\emptyset$ implying no priority value. From the Eve state $s_0$ add one transition $(s_0, s'_1, q, w)$ for every outgoing edge $(s_1, s'_1, p, w)$ of the Adam state $s_1$ where $p$ is the propositional formula annotating that edge, $w$ is the set of priority values for that edge, and $s'_1$ is another Adam edge, iff. fixing the uncontrollable APs in $p$ according to the evaluation $e_u$ results in a satisfiable propositional formula $q$. If $q$ is not satisfiable, continue the iteration. So you only add a transition if Eve can still make some move after Adam does.

If these new states and edges are added to the original automaton, then all original edges must be dropped, but the original states are retained.

This results in an automaton where the alternating turns between Adam an Eve in the parity game are made explicit.

### Zielonka

The function `hoax::HOAxParityTwA::solve_parity_game()` is a wrapper around `hoax::zielonka`. It returns the player that wins the parity game from the initial state according to the acceptance condition of the parity game. The caller should verify that the winning player, even (0) or odd (1) matches the player that is expected to win according to the acceptance condition. For example, if `hoax::HOAxParityTwA::solve_parity_game()` returns 0, the even player wins, but the acceptance condition is `parity max odd` meaning that the odd player is expected to win, then the parity game is _UNREALISABLE_.

Note that spot always encodes acceptance sets on the transitions, i.e. state based acceptance is implicit while transition-based acceptance is explicit. But, zielonka by definition makes use of state-based priorities. To bridge the gap, we simply derive state-based priority from transition-based priority, taking into account the `min/max` acceptance condition.

We define the priority of a transition to be the `min/max` of the acceptance sets that that transition is a part of. The priority of a state is then the `min/max` of the priorities of all of its outgoing transitions. We can pre-compute all priorities before calling zielonka.


# Dependencies

This section lists the project's dependencies, which **are required** to be set up before the project can be compiled an run.

Any installation instructions will target ubuntu linux, since that was the platform used for development.

## Meson

The meson build system is used for the compilation of the project.

```sh
# Install dependencies
sudo apt-get install meson
```

## Spot

The c++ library [Spot](https://spot.lre.epita.fr/index.html) implements LTL and $\omega$-automata, and facilitates their manipulation. Functionality of this library will be used, though we must try to implement some algorithms and formalisms ourselves.

For installation instructions, see [spot's official install instructions](https://spot.lre.epita.fr/install.html#tar) or follow the steps below.
For instructions on how to _"compile and execute a C++ program written using Spot"_, see [spot's official compilation instructions](https://spot.lre.epita.fr/compile.html).

### Install Spot

Let us install spot's development package. This does **not** allow us to modify the maximum number of acceptance sets that spot accepts.

```sh
# Install dependencies
sudo wget -q -O - https://www.lrde.epita.fr/repo/debian.gpg | sudo apt-key add -
echo 'deb http://www.lrde.epita.fr/repo/debian/ stable/' | sudo tee -a /etc/apt/sources.list
sudo apt-get update
sudo apt-get install libspot-dev
```

Alternatively, let us install spot from the spot 2.12.1 tarball, the most recent available version. Since this compiles from source, this **does** allow us to modify the maximum number of acceptance sets that spot accepts during the configure step.

```sh
# Download and extract the tarball
wget http://www.lrde.epita.fr/dload/spot/spot-2.12.1.tar.gz
tar -xf spot-2.12.1.tar.gz
cd spot-2.12.1/

# Perform the basic 'configure && make && make install' steps
./configure --disable-python
make
sudo make install

# Refresh default lib cache, which can resolve "libspot.so not found" errors
ldconfig -v
```

### Link Spot during compilation

Now verify the generated spot libs can be found and linked to the HOAx source code. Some of the examples spot provides for this purpose have been included in the repo as test cases; This project's meson test pipeline covers this. See [this section](#project---part-1-setup-of-the-parser).


For later reference, let us briefly describe the steps to test the linkage manually.

```sh
# Compile while linking spot with a test program.
g++ tests/test_spot_linkage.cpp -lspot -o output/hello
# Run the compiled test program.
./output/hello
# Cleanup
rm output/hello
```

At the time of writing, this writes the following to stdout:

    Test proper Spot Linkage!
    This is Spot 2.12.1.

### Useful Spot references

This section details some of the pages and files in the spot documentation that proved helpful.
First of all, [spot's install instructions](https://spot.lre.epita.fr/install.html#Debian).

Next, they provide additional, detailed installation instructions in the INSTALL file in the root of spot's cloned git repo. See also [their HACKING file](https://gitlab.lre.epita.fr/spot/spot/-/blob/next/HACKING) for configuration of their GIT tree, and [their README](https://gitlab.lre.epita.fr/spot/spot/-/blob/next/README) in the section `Troubleshooting installations` for the following commands.

```sh
# Refresh default lib cache
ldconfig -v
# Check if "make install" installed spot correctly
ltl2tgba --version
man spot
```

Next see [their compilation instructions](https://spot.lrde.epita.fr/compile.html), after the installation finished successfully.

# Compilation

The meson build system is used to compile the build targets.

Note that the `$LDFLAGS` environment variable must be set before linking occurs.
During development, I had some issues with linking spot AND my locally created HOAx library, likely due to my inexperience with a more complex build pipeline. Namely, meson was not ordering the linker arguments correctly: meson kept placing the `-lspot` and the `-Wl,--copy-dt-needed-entries` linker flags *behind* the locally created libraries during linking. This usually resulted in the following error.

```
/usr/bin/ld: /usr/local/lib/libbddx.so.0: error adding symbols: DSO missing from command line
```

The `build.sh` utility script takes care of project setup. But, this section provides more detail for the sake of completeness.
After cloning the git repo, we must setup meson at the **root of the project**.

```sh
# The very first time, call without "--wipe"
# Every time after that, prefer to add "--wipe".
# This ensures other meson commands display any warnings.

# Option (1): set the var for the current session
export LDFLAGS="-Wl,--copy-dt-needed-entries"
meson setup builddir/ --wipe

# Option (2); set the var only for this command
(export LDFLAGS="-Wl,--copy-dt-needed-entries" && meson setup builddir/ --wipe)
```

Next, testing is done by invoking the meson test command.

```sh
# The verbose flag "-v" is only necessary in case of test failures.
meson test -C builddir/ -v
```

Lastly, compile all build targets.

```sh
# Compile.
meson compile -C builddir/
# Run entrypoint.
./builddir/hoax
```

Each executable resulting from this compilation will appear in the subdirectories of `builddir/`, in a location corresponding to the `meson.build` file that generated that executable.


# Development

This section provides useful details for developing this project.

## HOA

This section discusses some details of the HOA format that matter w.r.t. solving parity games.
See the [official page](https://adl.github.io/hoaf/) of the HOA format.

### HOA Headers

Note the following [exerpts](https://adl.github.io/hoaf/#acceptance).

    Additionally the t and f Boolean constants can be used with their obvious meanings (t is always accepting while f is never accepting).

and

    An acceptance condition declaring m sets needs not actually use all of these sets. In this case the unused sets can be ignored if they appear in the body of the automaton.

This may be relevant for some of the benchmarks.

Note the following [exerpt](https://adl.github.io/hoaf/#canonical-acceptance-specifications-for-classical-conditions).

    The "Acceptance: line is what defines the acceptance semantics of the automaton and the "acc-name:" has only informative value.

This matters because spot indeed parses the `Acceptance` header to determine the acceptance conditions of the given parity automaton. So, in general, we should not rely on the `acc-name` header to define the acceptance condition.

## Spot documentation

Spot provides doxygen documentation online. [This page](https://spot.lre.epita.fr/doxygen/) lists some handy starting points in their docs, such as functions to parse a string into an LTL formula object.

Spot also provides a [list of examples](https://spot.lre.epita.fr/tut.html) in python and c++.

## eHOA

The extended HOA format used for this project was proposed in [this paper](https://arxiv.org/abs/1912.05793). It introduces a `controllable-AP` property in the HOA header section.

Thus, we require Spot to be able to parse and store this information.

**For c++**, just using `spot::parse_aut` for an input file seems to parse the `controllable-AP` property of the input file just fine.
The `spot::parse_aut` function returns an object of type `spot::parsed_aut_ptr`, which has a member `aut` of the type `spot::twa_graph_ptr`. The `aut` member contains the requisite `"synthesis-outputs"` named property, which is reflective of the `controllable-AP` eHOA property. Furthermore, dumping that automaton back out to HOA format shows that the `controllable-AP` property is conserved in the output file (at least when it was also part of the HOA file input).

### Spot-related eHOA & (Parity) Games information

On Spot's TωA page, the [section on arenas for two player games](https://spot.lre.epita.fr/hoa.html#orgcf37081) gives an example of an automaton that specifies `controllable-AP: 1`.

To access properties related to games, we may refer to the [named properties section](https://spot.lre.epita.fr/concepts.html#named-properties) of Spot's concepts page. The property `synthesis-outputs` seems to represent `controllable-AP` in this context, and related game properties are `state-winner`, `strategy` and `synthesis-outputs`.

To access any of these properties, see `twa::get_named_prop<type>(key)` in the spot [docs](https://spot.lre.epita.fr/doxygen/classspot_1_1twa.html#a38228ffd3a1bcde423b88a738fd86e98). In the SYNTCOMP benchmarks, none of the eHOA files seem to specify `spot-state-player: ...`, so the `"state-player"` named prop is always empty in the parsed automaton. Since we must implement our own synthesis solution, the `"state-winner"` and `"strategy"` named properties are likely also of no use. They encode information about the solution to a parity game, so they are probably set by spot's parity game solver, which we may obviously not call.

It does not seem like the struct `spot::twa_graph` makes the map of all properties public. So, if you *do* want to see what properties exist on a twa, then you could modify `/usr/local/include/spot/twa/twa.hh` (one of the header files of Spot installed by `make install`) so that the member `named_prop_` is public, by changing line 1081 from `protected:` to `public:`. Then, you can simply loop over the keys of the map.

```cpp
spot::twa_graph aut = ...;
std::cout << "Found properties: [";
// The member "aut->named_prop_" is the named property map containing
// the properties we are interested in.
for (const auto& key : aut->named_prop_) {
    std::cout << key.first << ", ";
}
std::cout << "]" << std::endl;

//==> e.g. outputs: "Found properties: [state-names, synthesis-outputs, ]"
```

The Spot tutorial [here](https://spot.lre.epita.fr/tut40.html
) gives a c++ code example of using Spot's game interface. It does not explicitly explain about the `controllable-AP` property.

[This Spot python example](https://spot.lre.epita.fr/ipynb/games.html) is about games. On a side note, it uses the function `get_state_players`. This appears to also be a function in the c++ interface, accessible via `#include <spot/twaalgos/game.hh>` as `spot::get_state_players`. Of note is the section [Input/Output in HOA format](https://spot.lre.epita.fr/ipynb/games.html#Input/Output-in-HOA-format) where they specify how to read eHOA via the pything bindings:

```py
game = spot.automaton("ltlsynt --ins=a --outs=b -f '!b & GFa <-> Gb' --print-game-hoa |");
```
