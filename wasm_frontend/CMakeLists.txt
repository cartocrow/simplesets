set(SOURCES
    simplesets.cpp
)

add_executable(simplesets_wasm ${SOURCES})

target_link_libraries(
    simplesets_wasm
    PRIVATE
    cartocrow::core
    cartocrow::circle_segment_helpers
    cartocrow::renderer
    simplesets
)
