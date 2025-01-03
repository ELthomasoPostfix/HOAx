test_ex = executable('spot_linkage',
    'test_spot_linkage.cpp',
    link_args : LSPOT_LINK_ARGS,
)
test('Test the proper installation and linkage of Spot.', test_ex)

test_ex = executable('spot_parser_api',
    'test_spot_parser_api.cpp',
    link_args : LSPOT_LINK_ARGS,
)
test('Test the availability of the Spot parser API.', test_ex,
    args:[DIR_IN, DIR_OUT])

test_ex = executable('spot_bdd_api',
    'test_spot_bdd_api.cpp',
    link_args : LSPOT_LINK_ARGS,
)
test('Test the availability of the Spot bdd API.', test_ex,
    args:[DIR_IN, DIR_OUT])
