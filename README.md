# HOAx

A repo containing projects 1 & 2 for the course Specification and Verification.

The goal is preparing a (HOA) parity game solver for the [SYNTCOMP](https://www.syntcomp.org/) 2025 competition.

This project is a **hoax**, since despite claiming to implement a parity game solver, it heavily relies on external libraries (Spot) to achieve this. A true computer scientist would reinvent the wheel.

# Dependencies

This sections describes the project's dependencies.

Any installation instructions will target ubuntu linux, since that was used for development.

## Spot

The c++ library [Spot](https://spot.lre.epita.fr/index.html) implements LTL and $\omega$-automata manipulation. Only select functionality of this library will be used, since we must implement several algorithms and formalisms ourselves.

For installation instructions, see [spot's install instructions](https://spot.lre.epita.fr/install.html#Debian) or do the following.
For compilation instructions, see [spot's compilation instructions](https://spot.lre.epita.fr/compile.html).
**I will compile from source using the [gitlab repo](https://gitlab.lre.epita.fr/spot/spot) they provide.**

### Install Spot dependencies

We first need to install spot's dependencies. This section may not be very coherent.
This is because setting up spot was a very winding road when I attempted it.

```sh
# Check dependency versions for bootstrapping
sudo apt-cache policy make autogen autoconf automake libtool flex bison emacs groff swig doxygen liblocale-gettext-perl imagemagick python3 ipython3 jupyter graphviz pdf2svg

# Install dependencies for bootstrapping
sudo apt satisfy "make, autogen, autoconf (>= 2.69), automake (>=1.11), libtool (>= 2.4), flex (>= 2.6), bison (>= 3.3), emacs (>= 24), groff (>= 1.20), swig (>= 4.0.1), doxygen (>= 1.4.0), liblocale-gettext-perl, imagemagick, python3 (>= 3.6), ipython3 (>= 2.3), jupyter (>= 4), graphviz, pdf2svg"

# Install more dependencies?
sudo apt-get install -y \
    wget git \
    emacs-nox emacs-goodies-el org-mode \
    make gcc g++ \
    swig3.0 \
    autoconf automake libtool \
    flex bison \
    graphviz imagemagick optipng \
    doxygen \
    help2man pandoc groff \
    r-base-core r-recommended- r-base-dev- r-base-html- \
    latexmk texlive-latex-extra texlive-fonts-extra \
    fonts-roboto \
    texlive-science \
    texlive-latex-extra-doc- texlive-fonts-extra-doc- \
    texlive-latex-base-doc- texlive-latex-recommended-doc- \
    texlive-pictures-doc- texlive-pstricks-doc- \
    texlive-fonts-recommended-doc- \
    devscripts git-buildpackage cmake \
    libbdd0c2 libbdd-dev zlib1g-dev \
    locales \
    default-jdk ant dot2tex pdf2svg lcov \
    g++-mingw-w64-i686 gcc-snapshot

# Clone repo
git clone https://gitlab.lre.epita.fr/spot/spot.git
cd spot

# Bootstrap GIT tree
# NOTE: make sure the path to the cloned spot git repo
# does not contain a directory with whitespaces in the name!
# e.g. /some/path/to/a/directory with whitespaces/spot
# contains a directory "directory with whitespaces".
# This will cause errors when running "autoreconf"!
autoreconf -vfi
./configure --disable-python
make
make install
```

### Link Spot during compilation

Now to try and link the generated libs for our own c++ program.
To test whether spot was installed correctly, and we can actually link it, we make use of one of the examples spot provides.

```sh
# Compile while linking spot, with a test program.
g++ tests/test_spot_linkage.cpp -lspot -o output/hello
# Run the compiled test program.
./output/hello
# Cleanup
rm output/hello
```

For me this output the following:

    Hello world!
    This is Spot 2.12.1.dev.

Note that a similar series of steps is part of the testing suite of this project's meson test pipeline. See [this section](#project---part-1-setup-of-the-parser).

### Useful Spot references

This section details some of the pages and files in the spot documentation that were helpful.
First of all, [spot's install instructions](https://spot.lre.epita.fr/install.html#Debian).

Next, they provide additional, detailed installation instructions in the INSTALL file in the root of spot's cloned git repo. See also [their HACKING file](https://gitlab.lre.epita.fr/spot/spot/-/blob/next/HACKING) for configuration of their GIT tree, and [their README](https://gitlab.lre.epita.fr/spot/spot/-/blob/next/README) in the section `Troubleshooting installations` for the following commands.

```sh
# Refresh default lib cache
ldconfig -v
# Check if "make install" installed spot correctly
ltl2tgba --version
man spot
```

Next see their [compilation instructions](https://spot.lrde.epita.fr/compile.html), after the installation finished successfully.


# Development

This section provides useful details for developing this project.

## Spot documentation

Spot provides doxygen documentation online. [This page](https://spot.lre.epita.fr/doxygen/) lists some handy starting points in the docs, such as functions to parse a string into an LTL formula object.

Spot also provides a [list of examples](https://spot.lre.epita.fr/tut.html) in python and c++.

## eHOA

The extended HOA format used for this project was propesed in [this paper](https://arxiv.org/abs/1912.05793). It introduces a `controllable-AP` property in the HOA header section.

Thus, we require Spot to be able to parse and store this information.

**For c++**, just using `spot::parse_aut` for an input file seems to parse the `controllable-AP` property of the input file just fine. The `spot::twa_graph_ptr` member `aut` of the `spot::parsed_aut_ptr` produced by the parsing contains the requisite `"synthesis-outputs"` named property, which is reflective of the `controllable-AP` property. Furthermore, dumping that automaton back out as HOA format shows that the `controllable-AP` prop is conserved (at least when it is part of the HOA file input).

### Spot-related eHOA & (Parity) Games information

On Spot's TωA page, the [section on arenas for two player games](https://spot.lre.epita.fr/hoa.html#orgcf37081) gives an example of an automaton that specifies `controllable-AP: 1`.

To access properties related to games, we may refer to the [named properties section](https://spot.lre.epita.fr/concepts.html#named-properties) of Spot's concepts page. The property `synthesis-outputs` seems to represent `controllable-AP` in this context, and related game properties are `state-winner`, `strategy` and `synthesis-outputs`.

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
) gives a c++ code example of solving a parity game using Spot, but it does not explain about the `controllable-AP` property.

[This Spot python example](https://spot.lre.epita.fr/ipynb/games.html) is about games. On a side note, it uses the function `get_state_players`. This appears to also be a function in the c++ interface, accessible via `#include <spot/twaalgos/game.hh>` as `spot::get_state_players`. Of note is the section [Input/Output in HOA format](https://spot.lre.epita.fr/ipynb/games.html#Input/Output-in-HOA-format) where they specify how to read eHOA via the pything bindings:

```py
game = spot.automaton("ltlsynt --ins=a --outs=b -f '!b & GFa <-> Gb' --print-game-hoa |");
```

# Run

This section explains how to run the project. This may be relevant to the evaluation of the project w.r.t. the course.

## Project - Part 1: Setup of the Parser

The first task for the project is setting up a parser that is able to read some input format and display it as a dot format image.
The meson build setup specifies several tests to verify this functionality.

First, [the linkage test](/tests/test_spot_linkage.cpp) verifies that Spot was installed correctly. If this script does not result in any compilation errors, and prints to the terminal, then all is well. The main concern is that you should explicitly link spot when compiling, as follows: `g++ ... -lspot -o ...`. The placement of `-lspot` in this command matters.

[The parser API test](/tests/test_spot_parser_api.cpp) shows that the Spot parser API exposes the expected functions. To provide visual indication of this, the test should also take some input file from the [input directory](/input/) and produce the dot image of the corresponding automaton into the [output directory](/output/). It also dumps the HOA of the automaton it parsed. This is to show that the `controllable-AP` property is preserved when an input automaton is specified in eHOA, meaning the input contains `controllable-AP`, since the output HOA file also specifies the original `controllable-AP` property value.


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
