set(SOURCES
    filtration_demo.cpp
    ../colors/colors.cpp
    ../widgets/double_slider.h
)

add_executable(filtration_demo ${SOURCES})

target_link_libraries(
    filtration_demo
    PRIVATE
    simplesets
    cartocrow::core
    cartocrow::renderer
    CGAL::CGAL
    Qt5::Widgets

)

install(TARGETS filtration_demo DESTINATION ${INSTALL_BINARY_DIR})
