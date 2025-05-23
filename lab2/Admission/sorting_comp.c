#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define TOTAL_SIZE 1000000
#define NUM_THREADS 4

typedef struct {
    int *data;
    int start;
    int end;
} ThreadData;

int cmp(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void* thread_sort(void *arg) {
    ThreadData *td = (ThreadData*)arg;
    qsort(td->data + td->start, td->end - td->start, sizeof(int), cmp);
    return NULL;
}

void merge(int *data, int start, int mid, int end) {
    int *temp = malloc((end - start) * sizeof(int));
    int i = start, j = mid, k = 0;
    
    while (i < mid && j < end) {
        if (data[i] < data[j]) {
            temp[k++] = data[i++];
        } else {
            temp[k++] = data[j++];
        }
    }
    
    while (i < mid) temp[k++] = data[i++];
    while (j < end) temp[k++] = data[j++];
    
    for (i = start, k = 0; i < end; i++, k++) {
        data[i] = temp[k];
    }
    
    free(temp);
}

int main() {
    int *data = malloc(TOTAL_SIZE * sizeof(int));
    int *qsorted = malloc(TOTAL_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < TOTAL_SIZE; i++) {
        data[i] = rand();
        qsorted[i] = data[i];
    }

    clock_t start_qsort = clock();
    qsort(qsorted, TOTAL_SIZE, sizeof(int), cmp);
    clock_t end_qsort = clock();
    printf("[qsort] Time: %.6f sec\n", 
           (double)(end_qsort - start_qsort) / CLOCKS_PER_SEC);

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int chunk_size = TOTAL_SIZE / NUM_THREADS;
    
    clock_t start_pt = clock();
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].data = data;
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == NUM_THREADS - 1) ? TOTAL_SIZE : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, thread_sort, &thread_data[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    for (int size = chunk_size; size < TOTAL_SIZE; size *= 2) {
        for (int i = 0; i < TOTAL_SIZE; i += 2 * size) {
            int mid = i + size;
            int end = (i + 2 * size < TOTAL_SIZE) ? i + 2 * size : TOTAL_SIZE;
            if (mid < end) {
                merge(data, i, mid, end);
            }
        }
    }
    
    clock_t end_pt = clock();
    printf("[Pthreads sort] Time: %.6f sec (threads: %d)\n", 
           (double)(end_pt - start_pt) / CLOCKS_PER_SEC, NUM_THREADS);

    for (int i = 1; i < TOTAL_SIZE; i++) {
        if (data[i] < data[i-1]) {
            printf("Sorting error at position %d\n", i);
            break;
        }
    }

    free(data);
    free(qsorted);
    return 0;
}