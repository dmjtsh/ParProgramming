#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WAVE_SPEED 1.0
#define TOTAL_TIME 1.0
#define SPACE_LENGTH 1.0
#define TIME_STEPS 1000
#define SPACE_POINTS 100
#define OUTPUT_SNAPSHOTS 9
#define SAVED_SNAPSHOTS 3

double initial_condition(double x) {
    return exp(-(x - SPACE_LENGTH / 2) * (x - SPACE_LENGTH / 2) * 100);
}

double boundary_condition(double t) {
    return 0.0;
}

double source_term(double t, double x) {
    return 0.0;
}

int main() {
    clock_t timer_start, timer_end;
    double execution_time;
    
    timer_start = clock();
    
    double time_step = TOTAL_TIME / TIME_STEPS;
    double space_step = SPACE_LENGTH / SPACE_POINTS;
    
    if (fabs(WAVE_SPEED) * time_step / space_step > 1.0) {
        printf("Stability warning: Courant number = %f\n", fabs(WAVE_SPEED) * time_step / space_step);
        printf("Solution stability not guaranteed. Adjust time_step or space_step.\n");
    }
    
    double **solution = (double **)malloc((TIME_STEPS + 1) * sizeof(double *));
    for (int step = 0; step <= TIME_STEPS; step++) {
        solution[step] = (double *)malloc((SPACE_POINTS + 1) * sizeof(double));
    }
    
    for (int point = 0; point <= SPACE_POINTS; point++) {
        solution[0][point] = initial_condition(point * space_step);
    }
    
    for (int step = 0; step <= TIME_STEPS; step++) {
        solution[step][0] = boundary_condition(step * time_step);
    }
    
    for (int point = 1; point < SPACE_POINTS; point++) {
        solution[1][point] = solution[0][point] - WAVE_SPEED * time_step / (2 * space_step) * 
                            (solution[0][point+1] - solution[0][point-1]) + 
                            time_step * source_term(0, point * space_step);
    }
    
    for (int step = 0; step <= TIME_STEPS; step++) {
        solution[step][SPACE_POINTS] = solution[step][SPACE_POINTS-1];
    }
    
    for (int time = 1; time < TIME_STEPS; time++) {
        for (int point = 1; point < SPACE_POINTS; point++) {
            solution[time+1][point] = solution[time-1][point] - 
                                    WAVE_SPEED * time_step / space_step * 
                                    (solution[time][point+1] - solution[time][point-1]) + 
                                    2 * time_step * source_term(time * time_step, point * space_step);
        }
        solution[time+1][SPACE_POINTS] = solution[time+1][SPACE_POINTS-1];
    }
    
    int snapshot_times[OUTPUT_SNAPSHOTS + 1];
    for (int i = 0; i <= OUTPUT_SNAPSHOTS; i++) {
        snapshot_times[i] = i * TIME_STEPS / OUTPUT_SNAPSHOTS;
    }
    
    FILE *output_file = fopen("wave_solution_data.csv", "w");
    if (output_file == NULL) {
        printf("File creation error\n");
        return EXIT_FAILURE;
    }
    
    fprintf(output_file, "x_position");
    for (int i = 0; i <= OUTPUT_SNAPSHOTS; i++) {
        fprintf(output_file, ",time_%d", i);
    }
    fprintf(output_file, "\n");
    
    for (int point = 0; point <= SPACE_POINTS; point++) {
        fprintf(output_file, "%.6f", point * space_step);
        
        for (int i = 0; i <= SAVED_SNAPSHOTS; i++) {
            int step = snapshot_times[i];
            fprintf(output_file, ",%.6f", solution[step][point]);
        }
        fprintf(output_file, "\n");
    }
    
    fclose(output_file);
    printf("Solution data written to wave_solution_data.csv\n");
    
    for (int step = 0; step <= TIME_STEPS; step++) {
        free(solution[step]);
    }
    free(solution);
    
    timer_end = clock();
    execution_time = ((double)(timer_end - timer_start)) / CLOCKS_PER_SEC;
    printf("Execution completed in %.4f seconds\n", execution_time);
    
    return EXIT_SUCCESS;
}