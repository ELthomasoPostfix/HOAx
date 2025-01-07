test_ex = executable('spot_linkage',
    'test_spot_linkage.cpp',
    dependencies : DEP_SPOT,
)
test('Test the proper installation and linkage of Spot.', test_ex)

test_ex = executable('spot_parser_api',
    'test_spot_parser_api.cpp',
    dependencies : DEP_SPOT,
)
test('Test the availability of the Spot parser API.', test_ex,
    args:[DIR_IN, DIR_OUT])

test_ex = executable('spot_bdd_api',
    'test_spot_bdd_api.cpp',
    dependencies : DEP_SPOT,
)
test('Test the availability of the Spot bdd API.', test_ex,
    args:[DIR_IN, DIR_OUT])

test_ex = executable('hoax_utils', files(
        'test_utils.cpp',
    ),
    include_directories : INC_HOAX,
    dependencies : DEP_SPOT,
    link_with : LIB_HOAX,
)
test('Test the HOAx utils.', test_ex,
    args:[DIR_IN])
