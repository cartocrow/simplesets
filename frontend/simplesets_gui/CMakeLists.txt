set(SOURCES
    simplesets_gui.cpp
    ../colors/colors.cpp
    ../widgets/double_slider.h
)

add_executable(simplesets_gui ${SOURCES})

target_link_libraries(
    simplesets_gui
    PRIVATE
    simplesets
    cartocrow::core
    cartocrow::renderer
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS simplesets_gui DESTINATION ${INSTALL_BINARY_DIR})

