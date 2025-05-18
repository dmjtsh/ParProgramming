#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank = 0;
    int size = 0;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            printf("Need minimum 2 threads.\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Iterations for Averaging
    const int iterations = 1000;  
    double total_latency = 0.0;

    for (int i = 0; i < iterations; ++i) {
        if (rank == 0) {
            double start_time = MPI_Wtime();

            MPI_Send(NULL, 0, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(NULL, 0, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            double end_time = MPI_Wtime();
            
            total_latency += (end_time - start_time);
        } 
        else if (rank == 1) {
            
            MPI_Recv(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
        
        }
    }

    if (rank == 0) {
        double avg_latency = (total_latency / (2 * iterations)) * 1e6;  // в микросекундах
        printf("Average delay: %.2f mks\n", avg_latency);
    }

    MPI_Finalize();
    return 0;
}