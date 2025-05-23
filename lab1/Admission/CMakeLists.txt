cmake_minimum_required(VERSION 3.12)
project(mpi_latency_measure C)

find_package(MPI REQUIRED)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(latency_measure latency_measure.c)
add_executable(calculate_pi calculate_pi.c)

target_link_libraries(latency_measure PRIVATE MPI::MPI_C)
target_link_libraries(calculate_pi PRIVATE MPI::MPI_C)