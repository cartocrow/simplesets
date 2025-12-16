set(TEST_SOURCES
    "simplesets_test.cpp"
    "collinear_island.cpp"
    "partition_algorithm.cpp"
    "poly_line_gon_intersection.cpp"
)

add_executable(simplesets_test simplesets_test.cpp ${TEST_SOURCES})
target_link_libraries(simplesets_test
    PRIVATE
    simplesets
)