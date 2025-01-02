project('HOAx', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3']
)

# Define constants
LSPOT_LINK_ARGS = ['-Wl,--copy-dt-needed-entries', '-std=c++17', '-lspot']
DIR_ROOT = meson.current_source_dir()
DIR_IN  = join_paths(DIR_ROOT, 'input')
DIR_OUT = join_paths(DIR_ROOT, 'output')

# Test directory.
subdir('tests')

# Build project's entry point executable.
hoax_ex = executable('hoax', files(
    'src/main.cpp',
    'src/hoax.cpp',
    'src/utils.cpp'
  ),
  link_args : LSPOT_LINK_ARGS)