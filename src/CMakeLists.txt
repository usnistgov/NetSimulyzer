# TODO: Find a better way than manually installing
set(OSG_DIR ${PROJECT_SOURCE_DIR}/lib/OpenSceneGraph/install)
find_package(OpenSceneGraph REQUIRED COMPONENTS osgViewer osgDB osgGA osgText osgUtil)

find_package(LibXml2 REQUIRED)

add_executable(visualization main.cpp parser/file-parser.cpp parser/file-parser.h parser/model.h)
target_compile_features(visualization PRIVATE cxx_std_17)

target_compile_options(visualization PRIVATE -Wall -Wextra -Wpedantic -pedantic-errors)

target_include_directories(visualization PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(visualization ${OPENSCENEGRAPH_LIBRARIES})

target_include_directories(visualization PRIVATE LibXml2::LibXml2)
target_link_libraries(visualization LibXml2::LibXml2)
