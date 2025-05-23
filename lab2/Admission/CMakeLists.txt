cmake_minimum_required(VERSION 3.12)
project(sorting_comp C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

find_package(OpenMP REQUIRED)

add_executable(sorting_comp sorting_comp.c)

target_link_libraries(sorting_comp PRIVATE 
    Threads::Threads
    OpenMP::OpenMP_C
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native")