# Meson setup.
# Exporting LDFLAGS is necessary for proper linking of spot.
(export LDFLAGS="-Wl,--copy-dt-needed-entries" && meson setup builddir/ --wipe)

# Meson testing.
# Run tests to verify the project works as intended on the local platform.
meson test -C builddir/

# Meson compilation
# Build the entry point (main) executable.
meson compile -C builddir/
