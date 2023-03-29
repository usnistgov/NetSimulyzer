add_library(QDarkStyle
    qdarkstyle/dark/darkstyle.qrc
    qdarkstyle/light/lightstyle.qrc
    )

target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(parser PRIVATE rapidjson)
