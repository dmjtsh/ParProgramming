#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int cmp(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int total_size = 1000000;
    int *data = NULL;
    int *qsorted = NULL;

    int local_size = total_size / size;
    int *local_data = malloc(local_size * sizeof(int));

    if (rank == 0) {
        data = malloc(total_size * sizeof(int));
        qsorted = malloc(total_size * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < total_size; i++) {
            data[i] = rand();
            qsorted[i] = data[i];
        }
    }

    // 1 thread qsort
    double start_qsort, end_qsort;
    if (rank == 0) {
        start_qsort = MPI_Wtime();
        qsort(qsorted, total_size, sizeof(int), cmp);
        end_qsort = MPI_Wtime();
        printf("[qsort] Time: %.6f sec\n", end_qsort - start_qsort);
    }

    // Assignment
    MPI_Scatter(data, local_size, MPI_INT,
                local_data, local_size, MPI_INT,
                0, MPI_COMM_WORLD);

 
    double start_mpi = MPI_Wtime();
    qsort(local_data, local_size, sizeof(int), cmp);

    // Gathering all
    int *result = NULL;
    if (rank == 0) result = malloc(total_size * sizeof(int));
    MPI_Gather(local_data, local_size, MPI_INT,
               result, local_size, MPI_INT,
               0, MPI_COMM_WORLD);

    double end_mpi = MPI_Wtime();

    if (rank == 0) {
        qsort(result, total_size, sizeof(int), cmp);
        printf("[MPI sort] Time: %.6f sec (processes: %d)\n", end_mpi - start_mpi, size);
    }

    if (rank == 0) {
        free(data);
        free(qsorted);
        free(result);
    }
    free(local_data);

    MPI_Finalize();
    return 0;
}