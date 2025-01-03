# HOAx

A repo containing the project for the course Specification and Verification.

The goal is preparing a (eHOA) parity game solver for the [SYNTCOMP](https://www.syntcomp.org/) 2025 competition.

This project is a **hoax**, since despite claiming to implement a parity game solver, it heavily relies on external libraries (Spot) to achieve this. A true computer scientist would reinvent the wheel.

A basic parity game solver consists of implementing zielonka's recursive algorithm. Note that spot has an [implementation](https://spot.lre.epita.fr/doxygen/group__games.html#ga5282822f1079cdefc43a1d1b0c83a024) of zielonka's algorithm as well, but, of course, we should **not** call it nor copy its code; we are to implement the algorithm ourselves.

For more details on the use of spot in this project, see [the dependencies section](#dependencies).


# Run

This section explains how to run the project. This may be relevant to the evaluation of the project w.r.t. the course.

**Alternatively, see the github actions [workflow yml](/.github/workflows/meson-actions.yml) for an ubuntu linux example of setting up and running the project.**

## Project - Setup

Before the project can be run, the dependencies must be setup. See the [dependencies section](#dependencies) on how to do that.

## Project - Part 1: Setup of the Parser

The first task for the project is setting up a parser that is able to read some input format and display it as a dot format image.
The meson build setup specifies several tests to verify this functionality.

First perform the [meson compilation](#compilation). Then the testing suite can be run. If it succeeds, then the parser works fine.

```sh
meson test -C builddir/ -v
```

[The linkage test](/tests/test_spot_linkage.cpp) verifies that Spot was installed correctly. If this script does not result in any compilation errors, and prints to the terminal, then all is well. The main concern is that you should explicitly link spot when compiling, as follows: `g++ ... -lspot -o ...`. The placement of `-lspot` in this command matters.

[The parser API test](/tests/test_spot_parser_api.cpp) shows that the Spot parser API exposes the expected functions. To provide visual indication of this, the test should also take some input file from the [input directory](/input/) and produce the dot image of the corresponding automaton into the [output directory](/output/). It also dumps the HOA of the automaton it parsed. This is to show that the `controllable-AP` property is preserved when an input automaton is specified in eHOA, meaning the input contains `controllable-AP`, since the output HOA file also specifies the original `controllable-AP` property value.

## Project - Part 2: Solve parity game realizability

Deciding **realizability** for a parity game means checking that player 0, Eve, has a winning strategy, which is equivalent with checking that player 1, Adam, does *not* have a winning strategy. This differs from **synthesis**, where you must not only decide realizability, but also output such a winning strategy in AIGER format.

### Run

To run the entrypoint for part two on the entire SYNTCOMP benchmark suite, run the following.

```sh
./run.sh

# Or a "brief" test of only the mucalc_mc subset of the benchmark
./run.sh --brief
```

### Implementation details

The basic implementation for solving parity game realizability is **Zielonka's algorithm**. This algorithm was described in the course document "playing games to synthesize reactive systems" in the section "A Divide-and-Conquer Algorithm for Parity Games". Pseudocode for the zielonka algorithm can also be found on the [wikipedia page](https://en.wikipedia.org/wiki/Parity_game#Recursive_algorithm_for_solving_parity_games) of parity games.


# Compilation

The meson build system is used to compile the build targets.
After cloning the git repo, we must setup meson at the **root of the project**.

```sh
# The very first time, call without "--wipe"
meson setup builddir/

# Every time after that, prefer to add "--wipe".
# This ensures other meson commands display any warnings.
meson setup builddir/ --wipe
```

Next, testing is done by invoking the meson test command.

```sh
# The verbose flag "-v" is only necessary in case of test failures.
meson test -C builddir/ -v
```

Lastly, you can compile all build targets at once. The meson build configuration was set up this way to make compilation as straight forward as possible.

```sh
# Compile.
meson compile -C builddir/
# Run entrypoint.
./builddir/hoax
```

Each executable resulting from this compilation will appear in the subdirectories of `builddir/`, in a location corresponding to the `meson.build` file that generated that executable.


# Dependencies

This section lists the project's dependencies, which **are required** to be set up before the project can be compiled an run.

Any installation instructions will target ubuntu linux, since that was the platform used for development.

## Spot

The c++ library [Spot](https://spot.lre.epita.fr/index.html) implements LTL and $\omega$-automata, and facilitates their manipulation. Functionality of this library will be used, though we must try to implement some algorithms and formalisms ourselves.

For installation instructions, see [spot's official install instructions](https://spot.lre.epita.fr/install.html#tar) or follow the steps below.
For instructions on how to _"compile and execute a C++ program written using Spot"_, see [spot's official compilation instructions](https://spot.lre.epita.fr/compile.html).

### Install Spot

Let us install spot from the spot 2.12.1 tarball, the most recent available version.

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


# Development

This section provides useful details for developing this project.

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
