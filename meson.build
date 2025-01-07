project('HOAx', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3']
)

# Define dependencies
cpp = meson.get_compiler('cpp')
DEP_SPOT = cpp.find_library('spot')

# Define constants
DIR_ROOT = meson.current_source_dir()
DIR_IN  = join_paths(DIR_ROOT, 'input')
DIR_OUT = join_paths(DIR_ROOT, 'output')

# HOAx source
subdir('src/hoax')
INC_HOAX = include_directories('src/hoax')

# Test directory.
subdir('tests')

# Build project's entry point executable.
hoax_ex = executable('hoax', files(
    'src/main.cpp',
  ),
  dependencies : DEP_SPOT,
  include_directories : INC_HOAX,
  link_with : LIB_HOAX)
