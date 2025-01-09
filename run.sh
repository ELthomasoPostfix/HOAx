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
    ./builddir/hoax -v -b input/parity/parity/mucalc_mc/*.ehoa
    ./builddir/hoax -v -b input/hoa_benchmarks/model_paper-2.property4-*.ehoa
else
    ./builddir/hoax -v input/parity/parity/keiren/*.ehoa
    ./builddir/hoax -v input/parity/parity/mucalc_mc/*.ehoa
    ./builddir/hoax -v input/parity/parity/pgsolver_based/*.ehoa
    ./builddir/hoax -v input/parity/parity/tlsf_based/*.ehoa
fi
