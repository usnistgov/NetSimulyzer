# NIST-developed software is provided by NIST as a public service. You may use,
# copy and distribute copies of the software in any medium, provided that you
# keep intact this entire notice. You may improve,modify and create derivative
# works of the software or any portion of the software, and you may copy and
# distribute such modifications or works. Modified works should carry a notice
# stating that you changed the software and should note the date and nature of
# any such change. Please explicitly acknowledge the National Institute of
# Standards and Technology as the source of the software.
#
# NIST-developed software is expressly provided "AS IS." NIST MAKES NO
# WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
# LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT
# AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE
# OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT
# ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY
# REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF,
# INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY,
# OR USEFULNESS OF THE SOFTWARE.
#
# You are solely responsible for determining the appropriateness of using and
# distributing the software and you assume all risks associated with its use,
# including but not limited to the risks and costs of program errors,
# compliance with applicable laws, damage to or loss of data, programs or
# equipment, and the unavailability or interruption of operation. This
# software is not intended to be used in any situation where a failure could
# cause risk of injury or damage to property. The software developed by NIST
# employees is not subject to copyright protection within the United States.
#
# Author: Evan Black <evan.black@nist.gov>

cmake_minimum_required(VERSION 3.13)
project(NetSimulyzer
        VERSION 1.0.11
        LANGUAGES C CXX)

# Flag indicating weather this is a tagged version or not
# Be sure to set to FALSE when tagging a release version
set(IS_DEV_VERSION TRUE)

set(ENABLE_DOXYGEN FALSE CACHE BOOL "Enable Doxygen for API documentation")

option(USE_BUNDLED_ASSIMP
    "Build 'assimp' from the bundled sources, rather than searching the OS"
    FALSE)

set(CMAKE_CXX_STANDARD 17)


# Assimp Settings
if (NOT USE_BUNDLED_ASSIMP)
    find_package(assimp 5.0.0)
endif ()

# Just in case they don't have Assimp installed.
#
# Make sure we've found the library, and the target
# just in case we found some older version without
# that target
if (NOT assimp_FOUND OR NOT TARGET assimp::assimp)
    option (ASSIMP_BUILD_TESTS "If the test suite for Assimp is built in addition to the library." ON)
    set(ASSIMP_BUILD_TESTS OFF)

    option(ASSIMP_NO_EXPORT "Disable Assimp's export functionality." OFF)
    set(ASSIMP_NO_EXPORT ON)

    option(BUILD_SHARED_LIBS "Build package with shared libraries." OFF)
    set(BUILD_SHARED_LIBS OFF)

    # STEP Importer seems to cause issues building on Windows
    option(ASSIMP_BUILD_STEP_IMPORTER "" ON)
    set(ASSIMP_BUILD_STEP_IMPORTER OFF)

    add_subdirectory(lib/assimp)
    # Suppress assimp warnings
    target_compile_options(assimp PRIVATE -w)
endif ()

add_subdirectory(lib/rapidjson)
add_subdirectory(parser)

find_package(Qt5 COMPONENTS Core Widgets Gui PrintSupport REQUIRED)
add_subdirectory(lib/QCustomPlot)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Necessary for QThreads & standard threads
find_package(Threads REQUIRED)

# Version and other info
configure_file(project.h.in
        ${PROJECT_BINARY_DIR}/project.h
        )

add_executable(netsimulyzer
        main.cpp
        netsimulyzer.rc
        lib/QDarkStyleSheet/qdarkstyle/dark/darkstyle.qrc
        lib/QDarkStyleSheet/qdarkstyle/light/lightstyle.qrc
        resources.qrc)

target_compile_features(netsimulyzer PRIVATE cxx_std_17)

target_compile_options(netsimulyzer PRIVATE -Wall -Wextra -Wpedantic -pedantic-errors)
target_compile_options(netsimulyzer PRIVATE -Wno-unknown-pragmas) # Disable warnings for IDE pragmas

if (IS_DEV_VERSION)
    target_compile_definitions(netsimulyzer PRIVATE NETSIMULYZER_DEVELOPMENT_VERSION)
endif ()

target_link_libraries(netsimulyzer PRIVATE parser)
target_link_libraries(netsimulyzer PRIVATE assimp)
target_include_directories(netsimulyzer PRIVATE lib/glm)
target_link_libraries(netsimulyzer PRIVATE QCustomPlot)
target_link_libraries(netsimulyzer PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui)
target_link_libraries(netsimulyzer PRIVATE Threads::Threads)

# For allowing cursor control on modern macOS
if (${APPLE})
    target_link_libraries(netsimulyzer PRIVATE "-framework ApplicationServices")
endif ()

add_subdirectory(src)

if(ENABLE_DOXYGEN)
    message(STATUS "Doxygen Enabled")
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen)

    find_package(Doxygen)
    doxygen_add_docs(doxygen-docs
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_SOURCE_DIR}/parser
            )

    add_custom_target(doc_doxygen ALL
            COMMAND Doxygen::doxygen Doxyfile.doxygen-docs
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation"
            VERBATIM
            )
endif()
