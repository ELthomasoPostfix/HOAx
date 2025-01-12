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

echo "[hoax] start at $(date +"%T")"
# The "--brief" arg is used to run the smallest benchmark subdir, for testing.
if [[ $* == *--brief* ]]
then
    # Modified versions of a mucalc_mc input, to see if the tool
    # crashes and burns if it sees anything other than "marity max even".
    time (./builddir/hoax -v -b input/hoa_benchmarks/model_paper-2.property4-*.ehoa)
    # A selection of keiren and tlsf_based inputs, which are actual instances of:
    #   a) "parity min odd"  with #ACCs = 2 or 3
    #   b) "parity max even" with #ACCs = 2 or 3
    time (./builddir/hoax -v -b input/hoa_benchmarks/diff-ACCs-*.ehoa)
    # The entire mucalc_mc subdir of the actual benchmarks.
    time (./builddir/hoax -v -b input/parity/parity/mucalc_mc/*.ehoa)
else
    # Run the entirety of the benchmarks. This requires the benchmarks to
    # be locally available at the relative path "./input/parity/parity/"
    time (./builddir/hoax -v input/parity/parity/keiren/*.ehoa)
    time (./builddir/hoax -v input/parity/parity/mucalc_mc/*.ehoa)
    time (./builddir/hoax -v input/parity/parity/pgsolver_based/*.ehoa)
    time (./builddir/hoax -v input/parity/parity/tlsf_based/*.ehoa)
fi
echo "[hoax] finished at $(date +"%T")"
