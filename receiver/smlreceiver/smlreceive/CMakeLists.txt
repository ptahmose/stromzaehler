# CMakeList.txt : CMake project for Middleware, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (Middleware "Middleware.cpp" "Middleware.h" "ReadMessage.h" "ReadMessage.cpp")

# TODO: Add tests and install targets if needed.
