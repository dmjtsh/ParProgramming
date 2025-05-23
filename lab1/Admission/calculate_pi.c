#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

double random_double() {
    return rand() / (RAND_MAX + 1.0);
}

int main(int argc, char *argv[]) {
    int my_rank, num_procs;
    long long total_samples = 1000000LL;
    long long global_count = 0LL;
    long long sum_samples = 0LL;
    double pi_estimate;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    srand(my_rank + time(NULL));
    
    long long local_samples = total_samples / num_procs;
    long long local_count = 0LL;

    for (long long i = 0; i < local_samples; ++i) {
        double x_coord = random_double();
        double y_coord = random_double();
        if (x_coord*x_coord + y_coord*y_coord < 1.0) {
            ++local_count;
        }
    }
    
    MPI_Reduce(&local_count, &global_count, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_samples, &sum_samples, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (my_rank == 0) {
        pi_estimate = 4.0 * global_count / sum_samples;
        printf("Approximated Pi value: %.6f\n", pi_estimate);
    }

    MPI_Finalize();
    
    return EXIT_SUCCESS;
}