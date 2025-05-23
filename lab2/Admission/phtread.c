#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdatomic.h>

#define ITERATIONS 10000
#define MESSAGE_SIZE 1024

typedef struct {
    char data[MESSAGE_SIZE];
    atomic_int ready;
    atomic_int done;
} shared_data_t;

shared_data_t shared;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_ready = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_done = PTHREAD_COND_INITIALIZER;

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void* thread_function(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        
        while (!atomic_load(&shared.ready)) {
            pthread_cond_wait(&cond_ready, &mutex);
        }
        
        atomic_store(&shared.done, 1);
        atomic_store(&shared.ready, 0);
        pthread_cond_signal(&cond_done);
        
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    char buffer[MESSAGE_SIZE];
    memset(buffer, 'A', MESSAGE_SIZE);
    
    atomic_init(&shared.ready, 0);
    atomic_init(&shared.done, 0);
    
    if (pthread_create(&thread, NULL, thread_function, NULL) != 0) {
        perror("Failed to create thread");
        return EXIT_FAILURE;
    }
    
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        memcpy(shared.data, buffer, MESSAGE_SIZE);
        atomic_store(&shared.ready, 1);
        pthread_cond_signal(&cond_ready);
        while (!atomic_load(&shared.done)) {
            pthread_cond_wait(&cond_done, &mutex);
        }
        atomic_store(&shared.done, 0);
        pthread_mutex_unlock(&mutex);
    }
    
    double start = get_time();
    
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        
        memcpy(shared.data, buffer, MESSAGE_SIZE);
        atomic_store(&shared.ready, 1);
        pthread_cond_signal(&cond_ready);
        
        while (!atomic_load(&shared.done)) {
            pthread_cond_wait(&cond_done, &mutex);
        }
        
        atomic_store(&shared.done, 0);
        pthread_mutex_unlock(&mutex);
    }
    
    double end = get_time();
    
    pthread_join(thread, NULL);
    
    printf("Thread Communication Benchmark:\n");
    printf("-----------------------------\n");
    printf("Iterations:     %d\n", ITERATIONS);
    printf("Message size:   %d bytes\n", MESSAGE_SIZE);
    printf("Total time:     %.6f seconds\n", end - start);
    printf("Average latency: %.3f microseconds\n", (end - start) * 1e6 / ITERATIONS);
    printf("Throughput:     %.2f MB/s\n", 
          (ITERATIONS * MESSAGE_SIZE * 2) / (1024.0 * 1024.0) / (end - start));
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_ready);
    pthread_cond_destroy(&cond_done);
    
    return EXIT_SUCCESS;
}