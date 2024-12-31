# meson setup builddir/ --wipe
meson compile -C builddir/

# The "brief" arg is used to run the smallest benchmark subdir, for testing.
if [[ $* == *--brief* ]]
then
    ./builddir/hoax input/parity/parity/mucalc_mc/*.ehoa
else
    ./builddir/hoax -v input/parity/parity/keiren/*.ehoa
    ./builddir/hoax -v input/parity/parity/mucalc_mc/*.ehoa
    ./builddir/hoax -v input/parity/parity/pgsolver_based/*.ehoa
    ./builddir/hoax -v input/parity/parity/tlsf_based/*.ehoa
fi
