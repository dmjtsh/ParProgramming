cmake_minimum_required(VERSION 3.12)
project(mpi_lab1 C)

find_package(MPI REQUIRED)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(lab1 lab1.c)
add_executable(lab1_mpi lab1_mpi.c)

target_link_libraries(lab1 PRIVATE MPI::MPI_C m)
target_link_libraries(lab1_mpi PRIVATE MPI::MPI_C m)