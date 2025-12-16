set(SOURCES
    simplesets_cl.cpp
)

add_executable(simplesets_cl ${SOURCES})

find_package(nlohmann_json REQUIRED)

target_link_libraries(
    simplesets_cl
    PRIVATE
    simplesets
    cartocrow::core
    cartocrow::reader
    cartocrow::renderer
    CGAL::CGAL
    nlohmann_json::nlohmann_json
)

install(TARGETS simplesets_cl DESTINATION ${INSTALL_BINARY_DIR})