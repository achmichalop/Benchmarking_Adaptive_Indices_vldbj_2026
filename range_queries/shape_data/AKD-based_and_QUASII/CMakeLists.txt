cmake_minimum_required(VERSION 3.10)
project(MultidimensionalAdaptiveIndex)
set (CMAKE_CXX_STANDARD 14)
add_definitions("-Wall -Wextra -Wpedantic -O3 -march=native")

# generate a compilation database
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Setting output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Header files
include_directories(${CMAKE_SOURCE_DIR}/include/helpers)
include_directories(${CMAKE_SOURCE_DIR}/include/indexes)
include_directories(${CMAKE_SOURCE_DIR}/include/indexes/kd_tree)
include_directories(${CMAKE_SOURCE_DIR}/include/indexes/quasii)
include_directories(${CMAKE_SOURCE_DIR}/include/third_party)

set(SOURCES ${CMAKE_SOURCE_DIR}/src/helpers/column.cpp
            ${CMAKE_SOURCE_DIR}/src/helpers/cost_model.cpp
            ${CMAKE_SOURCE_DIR}/src/helpers/measurements.cpp
            ${CMAKE_SOURCE_DIR}/src/helpers/predicate.cpp
            ${CMAKE_SOURCE_DIR}/src/helpers/query.cpp
            ${CMAKE_SOURCE_DIR}/src/helpers/workload.cpp
            ${CMAKE_SOURCE_DIR}/src/helpers/table.cpp
            ${CMAKE_SOURCE_DIR}/src/indexes/full_scan.cpp
            ${CMAKE_SOURCE_DIR}/src/indexes/full_scan_candidate_list.cpp
            ${CMAKE_SOURCE_DIR}/src/indexes/full_scan_slow.cpp
            ${CMAKE_SOURCE_DIR}/src/indexes/progressive_indexing.cpp
            ${CMAKE_SOURCE_DIR}/src/indexes/cracking_kd_tree.cpp
            ${CMAKE_SOURCE_DIR}/src/indexes/quasii/slice.cpp
            ${CMAKE_SOURCE_DIR}/src/indexes/quasii.cpp
            ${CMAKE_SOURCE_DIR}/src/indexes/kd_tree/kd_tree.cpp
            ${CMAKE_SOURCE_DIR}/src/indexes/kd_tree/kd_node.cpp
        )


add_library(MDAI_lib ${SOURCES})

add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(main MDAI_lib)
