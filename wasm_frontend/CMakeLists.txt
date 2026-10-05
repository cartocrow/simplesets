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

target_link_options(simplesets_wasm PRIVATE
    --emit-tsd "$<TARGET_FILE_DIR:simplesets_wasm>/simplesets_wasm.d.ts"
    -sMODULARIZE -sEXPORT_ES6
        -g
        -gsource-map
        -sASSERTIONS
)
