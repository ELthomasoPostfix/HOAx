# HOAx

A repo containing projects 1 & 2 for the course Specification and Verification.

The goal is preparing a (HOA) parity game solver for the [SYNTCOMP](https://www.syntcomp.org/) 2025 competition.

# Dependencies

This sections describes the project's dependencies.

Any installation instructions will target ubuntu linux, since that was used for development.

## Spot

The c++ library [Spot](https://spot.lre.epita.fr/index.html) implements LTL and $\omega$-automata manipulation. Only select functionality of this library will be used, since we must implement several algorithms and formalisms ourselves. Of use are the following features:
1) The automaton parser
2) ltlcross and autcross for benchmarking

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
g++ -std=c++17 src/spot/hello.cpp -lspot -o output/hello
# Run the compiled test program.
./output/hello
```

For me this output the following:

    Hello world!
    This is Spot 2.12.1.dev.

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
