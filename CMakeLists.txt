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
project(visualization
        VERSION 0.1.0
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

SET(ASSIMP_BUILD_TESTS CACHE BOOL OFF)
SET(ASSIMP_NO_EXPORT CACHE BOOL ON)
SET(BUILD_SHARED_LIBS CACHE BOOL OFF)
add_subdirectory(lib/assimp)
# Suppress assimp warnings
target_compile_options(assimp PRIVATE -w)

add_subdirectory(lib/json)
add_subdirectory(lib/stb)
add_subdirectory(parser)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core Widgets Gui Charts REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Necessary for QThreads & standard threads
find_package(Threads REQUIRED)

# Version and other info
configure_file(project.h.in
        ${PROJECT_BINARY_DIR}/project.h
        )

add_executable(visualization
        settings.h
        main.cpp
        visualization.rc
        resources.qrc)

target_compile_features(visualization PRIVATE cxx_std_17)

target_compile_options(visualization PRIVATE -Wall -Wextra -Wpedantic -pedantic-errors)

target_link_libraries(visualization PRIVATE parser)
target_link_libraries(visualization PRIVATE assimp)
target_link_libraries(visualization PRIVATE stb)
target_include_directories(visualization PRIVATE lib/glm)
target_link_libraries(visualization PRIVATE Qt5::Core Qt5::Widgets Qt5::Charts Qt5::Gui)
target_link_libraries(visualization PRIVATE Threads::Threads)

add_subdirectory(src)
