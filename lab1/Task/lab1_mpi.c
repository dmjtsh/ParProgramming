#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define NX 100
#define NT 200
#define A 1.0
#define X_MAX 2.0
#define T_MAX 1.0

double phi(double x) {
    return exp(-x * x);
}

double psi(double t) {
    return 1.0 / (1.0 + t * t);
}

double f(double t, double x) {
    return 0.0;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double h = X_MAX / (NX - 1);
    double tau = T_MAX / (NT - 1);

    // Recognize part of every thread
    int base = NX / size;
    int extra = (rank < NX % size) ? 1 : 0;
    int local_NX = base + extra;

    int offset = rank * base + (rank < NX % size ? rank : NX % size);

    // Time and X
    double u[NT][local_NX];

    // Start condtion
    for (int m = 0; m < local_NX; m++) {
        double x = (offset + m) * h;
        u[0][m] = phi(x);
    }

    for (int k = 0; k < NT; k++) {
        double t = k * tau;
        if (rank == 0)
            u[k][0] = psi(t);
    }

    for (int k = 0; k < NT - 1; k++) {
        double left_value;
        if (rank != 0 && local_NX > 0) {
            MPI_Sendrecv(&u[k][0], 1, MPI_DOUBLE, rank - 1, 0,
                         &left_value, 1, MPI_DOUBLE, rank - 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for (int m = 0; m < local_NX; m++) {
            double x = (offset + m) * h;
            double u_left = (m == 0) ? (rank == 0 ? u[k][m] : left_value) : u[k][m - 1];
            u[k + 1][m] = u[k][m] - A * (tau / h) * (u[k][m] - u_left) + tau * f(k * tau, x);
        }
    }

    if (rank == 0) {
        FILE *fp = fopen("result_mpi.txt", "w");

        for (int k = 0; k < NT; k++) {

            for (int m = 0; m < local_NX; m++)
                fprintf(fp, "%.6f ", u[k][m]);

            for (int p = 1; p < size; p++) {
                int recv_NX = NX / size + (p < NX % size ? 1 : 0);
                double *recv_buf = malloc(recv_NX * sizeof(double));
                MPI_Recv(recv_buf, recv_NX, MPI_DOUBLE, p, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                for (int m = 0; m < recv_NX; m++)
                    fprintf(fp, "%.6f ", recv_buf[m]);

                free(recv_buf);
            }

            fprintf(fp, "\n");
        }

        fclose(fp);
        printf("Result saved in result_mpi.txt\n");
    } else {

        for (int k = 0; k < NT; k++) {
            MPI_Send(&u[k][0], local_NX, MPI_DOUBLE, 0, k, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
