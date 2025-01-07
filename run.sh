# The "--compile" arg is used to recompile the entrypoint.
# This is for ease of use during development.
if [[ $* == *--compile* ]]
then
    meson compile -C builddir/

    # Quit if compilation failed!
    if [ $? -ne 0 ]; then
        echo "HOAx: Failed compilation, exit ..."
        exit 1
    fi
fi

# The "--brief" arg is used to run the smallest benchmark subdir, for testing.
if [[ $* == *--brief* ]]
then
    # ./builddir/hoax -v input/parity/parity/mucalc_mc/*.ehoa

    FILE="toy_example_2.ehoa"
    ./builddir/hoax "input/hoa_benchmarks/$FILE"
    dot -Tpng "output/$FILE.dot" > "output/$FILE.png"
    dot -Tpng "output/${FILE}_EXP.dot" > "output/${FILE}_EXT.png"
else
    ./builddir/hoax -v input/parity/parity/keiren/*.ehoa
    ./builddir/hoax -v input/parity/parity/mucalc_mc/*.ehoa
    ./builddir/hoax -v input/parity/parity/pgsolver_based/*.ehoa
    ./builddir/hoax -v input/parity/parity/tlsf_based/*.ehoa
fi
