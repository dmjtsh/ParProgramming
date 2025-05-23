#include <stdio.h>
#include <mpi.h>
#include <string.h>

#define ITERATIONS 10000
#define MESSAGE_SIZE 1024
#define WARMUP_ITERATIONS 100

int main(int argc, char* argv[]) {
    int rank, size;
    char buffer[MESSAGE_SIZE];
    double start_time, end_time;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (size != 2) {
        if (rank == 0) {
            fprintf(stderr, "Error: This benchmark requires exactly 2 MPI processes\n");
            fprintf(stderr, "Usage: mpirun -np 2 %s\n", argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    
    memset(buffer, 'A' + rank, MESSAGE_SIZE);
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        if (rank == 0) {
            MPI_Send(buffer, MESSAGE_SIZE, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(buffer, MESSAGE_SIZE, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
        } else {
            MPI_Recv(buffer, MESSAGE_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Send(buffer, MESSAGE_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    for (int i = 0; i < ITERATIONS; i++) {
        if (rank == 0) {
            MPI_Send(buffer, MESSAGE_SIZE, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(buffer, MESSAGE_SIZE, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
        } else {
            MPI_Recv(buffer, MESSAGE_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Send(buffer, MESSAGE_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        }
    }
    
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        double total_time = end_time - start_time;
        double avg_latency = (total_time * 1e6) / ITERATIONS;
        double throughput = (ITERATIONS * MESSAGE_SIZE * 2) / (1024.0 * 1024.0) / total_time;
        
        printf("MPI Ping-Pong Benchmark Results:\n");
        printf("-------------------------------\n");
        printf("Message size:          %d bytes\n", MESSAGE_SIZE);
        printf("Iterations:            %d\n", ITERATIONS);
        printf("Total time:            %.6f seconds\n", total_time);
        printf("Average latency:       %.3f microseconds\n", avg_latency);
        printf("Throughput:            %.2f MB/s\n", throughput);
    }
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}