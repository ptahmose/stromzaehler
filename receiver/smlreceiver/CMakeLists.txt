# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("smlreceiver")

include(FetchContent)

message(STATUS "Fetching cxxopts")
FetchContent_Declare(cxxopts URL https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.0.0.zip)

# Include sub-projects.
add_subdirectory ("smlreceive")
