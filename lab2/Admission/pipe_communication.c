#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MESSAGE_SIZE 1024  
#define ITERATIONS 10000   

int main() {
    int pipefd[2];
    char message[MESSAGE_SIZE];
    struct timespec start, end;
    double total_time = 0.0;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  /
        close(pipefd[1]);
        for (int i = 0; i < ITERATIONS; i++) {
            read(pipefd[0], message, MESSAGE_SIZE);
        }

        close(pipefd[0]);
    } else { 
        close(pipefd[0]);  

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < ITERATIONS; i++) {
            write(pipefd[1], message, MESSAGE_SIZE);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        total_time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);

        close(pipefd[1]);
        wait(NULL); 

        double avg_time = (total_time / ITERATIONS) / 1e3;  
        printf("Avg time pipe: %.2f mks\n", avg_time);
    }

    return 0;
}