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

    if (pipe(pipefd) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // Дочерний процесс (получатель)
        close(pipefd[1]);  // Закрываем запись в pipe

        for (int i = 0; i < ITERATIONS; i++) {
            read(pipefd[0], message, MESSAGE_SIZE);
        }

        close(pipefd[0]);
    } else {  // Родительский процесс (отправитель)
        close(pipefd[0]);  // Закрываем чтение из pipe

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < ITERATIONS; i++) {
            write(pipefd[1], message, MESSAGE_SIZE);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        total_time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);

        close(pipefd[1]);
        wait(NULL);  // Ожидаем завершения дочернего процесса

        double avg_time = (total_time / ITERATIONS) / 1e3;  // В микросекундах
        printf("Среднее время передачи через pipe: %.2f мкс\n", avg_time);
    }

    return 0;
}