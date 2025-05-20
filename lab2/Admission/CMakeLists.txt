cmake_minimum_required(VERSION 3.12)
project(sorting_comp C)

find_package(MPI REQUIRED)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(sorting_comp sorting_comp.c)

target_link_libraries(sorting_comp PRIVATE MPI::MPI_C)