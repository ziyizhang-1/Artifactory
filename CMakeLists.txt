cmake_minimum_required(VERSION 3.29)
project(cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER g++)

include_directories(include)
include_directories("/opt/homebrew/opt/libomp/include")

set(OpenMP_CXX_FLAGS "-fopenmp")
set(OpenMP_CXX_LIB_NAMES libomp)
set(OpenMP_libomp_LIBRARY /opt/homebrew/opt/libomp/lib)

find_package(OpenMP REQUIRED)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS})

add_executable(
        stp
        src/SmartTypePriority.cpp
)

target_link_libraries(
        stp
        PUBLIC
        OpenMP::OpenMP_CXX
)

include(CMakePrintHelpers)
cmake_print_properties(
        TARGETS
        OpenMP::OpenMP_CXX
        PROPERTIES
        INTERFACE_COMPILE_OPTIONS
        INTERFACE_INCLUDE_DIRECTORIES
        INTERFACE_LINK_LIBRARIES
)
