#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define WAVE_SPEED 1.0
#define SIM_TIME 1.0
#define DOMAIN_LENGTH 1.0
#define TIME_STEPS 10000
#define GRID_POINTS 10000
#define OUTPUT_STEPS 9
#define SAVE_STEPS 3

double init_cond(double x) {
    return exp(-(x - DOMAIN_LENGTH / 2) * (x - DOMAIN_LENGTH / 2) * 100);
}

double bound_cond(double t) {
    return 0.0;
}

double src_term(double t, double x) {
    return 0.0;
}

int main(int argc, char *argv[]) {
    int proc_rank, num_procs;
    double t_start, t_end;
    double dt = SIM_TIME / TIME_STEPS;
    double dx = DOMAIN_LENGTH / GRID_POINTS;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    if (proc_rank == 0) {
        t_start = MPI_Wtime();
        if (fabs(WAVE_SPEED) * dt / dx > 1.0) {
            printf("Stability warning: CFL = %f\n", fabs(WAVE_SPEED) * dt / dx);
        }
    }

    int local_size = (GRID_POINTS + 1) / num_procs;
    int extra = (GRID_POINTS + 1) % num_procs;
    
    int local_start = proc_rank * local_size;
    if (proc_rank < extra) {
        local_size++;
        local_start += proc_rank;
    } else {
        local_start += extra;
    }

    int has_left_ghost = (proc_rank > 0);
    int has_right_ghost = (proc_rank < num_procs - 1);
    int buffer_size = local_size + has_left_ghost + has_right_ghost;

    double *u_old = malloc(buffer_size * sizeof(double));
    double *u_cur = malloc(buffer_size * sizeof(double));
    double *u_new = malloc(buffer_size * sizeof(double));

    for (int i = 0; i < buffer_size; i++) {
        int global_idx = local_start + i - has_left_ghost;
        if (i >= has_left_ghost && i < buffer_size - has_right_ghost) {
            u_old[i] = init_cond(global_idx * dx);
        }
    }

    if (proc_rank == 0) {
        u_old[has_left_ghost] = bound_cond(0);
    }

    double send_val, recv_val;
    MPI_Status status;

    if (proc_rank > 0) {
        send_val = u_old[has_left_ghost];
        MPI_Sendrecv(&send_val, 1, MPI_DOUBLE, proc_rank-1, 0,
                    &recv_val, 1, MPI_DOUBLE, proc_rank-1, 0,
                    MPI_COMM_WORLD, &status);
        u_old[0] = recv_val;
    }

    if (proc_rank < num_procs - 1) {
        send_val = u_old[buffer_size - has_right_ghost - 1];
        MPI_Sendrecv(&send_val, 1, MPI_DOUBLE, proc_rank+1, 0,
                    &recv_val, 1, MPI_DOUBLE, proc_rank+1, 0,
                    MPI_COMM_WORLD, &status);
        u_old[buffer_size - 1] = recv_val;
    }

    for (int i = has_left_ghost; i < buffer_size - has_right_ghost; i++) {
        if (i == has_left_ghost && proc_rank == 0) {
            u_cur[i] = bound_cond(dt);
        } else {
            int global_idx = local_start + i - has_left_ghost;
            double x_pos = global_idx * dx;
            u_cur[i] = u_old[i] - WAVE_SPEED * dt / (2 * dx) * 
                       (u_old[i+1] - u_old[i-1]) + dt * src_term(0, x_pos);
        }
    }

    double *global_data = NULL;
    int *counts = NULL;
    int *offsets = NULL;

    if (proc_rank == 0) {
        global_data = malloc((GRID_POINTS + 1) * sizeof(double));
        counts = malloc(num_procs * sizeof(int));
        offsets = malloc(num_procs * sizeof(int));

        int offset = 0;
        for (int p = 0; p < num_procs; p++) {
            counts[p] = (GRID_POINTS + 1) / num_procs;
            if (p < extra) counts[p]++;
            offsets[p] = offset;
            offset += counts[p];
        }
    }

    MPI_Gatherv(&u_old[has_left_ghost], local_size, MPI_DOUBLE,
               global_data, counts, offsets, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    double **time_snapshots = NULL;
    if (proc_rank == 0) {
        time_snapshots = malloc((OUTPUT_STEPS + 1) * sizeof(double *));
        for (int s = 0; s <= OUTPUT_STEPS; s++) {
            time_snapshots[s] = malloc((GRID_POINTS + 1) * sizeof(double));
        }
        for (int i = 0; i <= GRID_POINTS; i++) {
            time_snapshots[0][i] = global_data[i];
        }
    }

    for (int step = 1; step < TIME_STEPS; step++) {
        if (proc_rank > 0) {
            send_val = u_cur[has_left_ghost];
            MPI_Sendrecv(&send_val, 1, MPI_DOUBLE, proc_rank-1, 0,
                        &recv_val, 1, MPI_DOUBLE, proc_rank-1, 0,
                        MPI_COMM_WORLD, &status);
            u_cur[0] = recv_val;
        }

        if (proc_rank < num_procs - 1) {
            send_val = u_cur[buffer_size - has_right_ghost - 1];
            MPI_Sendrecv(&send_val, 1, MPI_DOUBLE, proc_rank+1, 0,
                        &recv_val, 1, MPI_DOUBLE, proc_rank+1, 0,
                        MPI_COMM_WORLD, &status);
            u_cur[buffer_size - 1] = recv_val;
        }

        for (int i = has_left_ghost; i < buffer_size - has_right_ghost; i++) {
            if (i == has_left_ghost && proc_rank == 0) {
                u_new[i] = bound_cond((step+1)*dt);
            } else {
                int global_idx = local_start + i - has_left_ghost;
                double x_pos = global_idx * dx;
                u_new[i] = u_old[i] - WAVE_SPEED * dt / dx * 
                          (u_cur[i+1] - u_cur[i-1]) + 
                          2 * dt * src_term(step*dt, x_pos);
            }
        }

        if (step % (TIME_STEPS / OUTPUT_STEPS) == 0) {
            int snap_idx = step / (TIME_STEPS / OUTPUT_STEPS);
            MPI_Gatherv(&u_new[has_left_ghost], local_size, MPI_DOUBLE,
                       global_data, counts, offsets, MPI_DOUBLE,
                       0, MPI_COMM_WORLD);
            
            if (proc_rank == 0) {
                for (int i = 0; i <= GRID_POINTS; i++) {
                    time_snapshots[snap_idx][i] = global_data[i];
                }
            }
        }

        double *tmp = u_old;
        u_old = u_cur;
        u_cur = u_new;
        u_new = tmp;
    }

    if (proc_rank == 0) {
        FILE *out = fopen("wave_solution_mpi.csv", "w");
        if (out) {
            fprintf(out, "x");
            for (int s = 0; s <= OUTPUT_STEPS; s++) {
                fprintf(out, ",t_%d", s);
            }
            fprintf(out, "\n");

            for (int i = 0; i <= GRID_POINTS; i++) {
                fprintf(out, "%f", i*dx);
                for (int s = 0; s <= SAVE_STEPS; s++) {
                    fprintf(out, ",%f", time_snapshots[s][i]);
                }
                fprintf(out, "\n");
            }
            fclose(out);
        }

        t_end = MPI_Wtime();
        double runtime = t_end - t_start;
        printf("Runtime: %.4f sec with %d procs\n", runtime, num_procs);

        FILE *log = fopen("mpi_perf.log", "a");
        if (log) {
            fprintf(log, "%d,%.4f\n", num_procs, runtime);
            fclose(log);
        }

        for (int s = 0; s <= OUTPUT_STEPS; s++) {
            free(time_snapshots[s]);
        }
        free(time_snapshots);
        free(global_data);
        free(counts);
        free(offsets);
    }

    free(u_old);
    free(u_cur);
    free(u_new);

    MPI_Finalize();
    return 0;
}