cmake_minimum_required(VERSION 3.12)
project(mpi_lab2 C)

find_package(MPI REQUIRED)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(lab2 sin.c)
target_link_libraries(lab2 PRIVATE MPI::MPI_C m)