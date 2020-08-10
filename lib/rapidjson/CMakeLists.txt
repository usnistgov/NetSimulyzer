cmake_policy(SET CMP0076 NEW)
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE .)
