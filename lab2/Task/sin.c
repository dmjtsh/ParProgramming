#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

typedef struct Task 
{
    double a;
    double b;
    double eps;
    struct Task* next;
} 
Task;

typedef struct 
{
    Task* head;
    Task* tail;
} 
Queue;

Queue* queue_init() {
    Queue* q = malloc(sizeof(Queue));
    q->head = q->tail = NULL;
    return q;
}

void enqueue(Queue* q, Task* task) {
    task->next = NULL;
    if (q->tail) {
        q->tail->next = task;
        q->tail = task;
    } else {
        q->head = q->tail = task;
    }
}

Task* dequeue(Queue* q) {
    if (!q->head) return NULL;
    Task* task = q->head;
    q->head = task->next;
    if (!q->head) q->tail = NULL;
    return task;
}

static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
static Queue* queue = NULL;
static double total_sum = 0.0;
static pthread_mutex_t sum_mutex = PTHREAD_MUTEX_INITIALIZER;
static int done = 0;
static int active_threads = 0;
static pthread_mutex_t active_mutex = PTHREAD_MUTEX_INITIALIZER;

void* worker(void* arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&queue_mutex);
        
        while (queue->head == NULL && !done) {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }
        
        if (done && queue->head == NULL) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }
        Task* task = dequeue(queue);
        pthread_mutex_unlock(&queue_mutex);

        if (!task) continue;

        pthread_mutex_lock(&active_mutex);
        active_threads++;
        pthread_mutex_unlock(&active_mutex);

        double a = task->a;
        double b = task->b;
        double eps = task->eps;
        free(task);

        double mid = (a + b) / 2.0;
        double h = b - a;
        double simple = h * (sin(1.0 / a) + sin(1.0 / b)) / 2.0;
        double mid_value = sin(1.0 / mid);
        double detailed = (h/2.0) * (sin(1.0 / a) + 2.0 * mid_value + sin(1.0 / b)) / 2.0;
        double error = fabs(simple - detailed);

        if (error < eps) {
            pthread_mutex_lock(&sum_mutex);
            total_sum += detailed;
            pthread_mutex_unlock(&sum_mutex);
        } else {
            Task* t1 = malloc(sizeof(Task));
            Task* t2 = malloc(sizeof(Task));
            t1->a = a; t1->b = mid; t1->eps = eps / 2; t1->next = NULL;
            t2->a = mid; t2->b = b; t2->eps = eps / 2; t2->next = NULL;

            pthread_mutex_lock(&queue_mutex);
            enqueue(queue, t1);
            enqueue(queue, t2);
            pthread_cond_broadcast(&queue_cond);
            pthread_mutex_unlock(&queue_mutex);
        }

        pthread_mutex_lock(&active_mutex);
        active_threads--;
        if (active_threads == 0) {
            pthread_mutex_lock(&queue_mutex);
            if (queue->head == NULL) {
                done = 1;
                pthread_cond_broadcast(&queue_cond);
            }
            pthread_mutex_unlock(&queue_mutex);
        }
        pthread_mutex_unlock(&active_mutex);
    }
    
    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s a b num_threads epsilon\n", argv[0]);
        return 1;
    }

    double a = atof(argv[1]);
    double b = atof(argv[2]);
    int num_threads = atoi(argv[3]);
    double eps = atof(argv[4]);

    queue = queue_init();
    Task* initial_task = malloc(sizeof(Task));
    initial_task->a = a;
    initial_task->b = b;
    initial_task->eps = eps;
    initial_task->next = NULL;
    enqueue(queue, initial_task);

    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("Integral value: %.12f\n", total_sum);

    pthread_mutex_lock(&queue_mutex);
    Task* task;
    while ((task = dequeue(queue)) != NULL) {
        free(task);
    }
    pthread_mutex_unlock(&queue_mutex);
    free(queue);

    return 0;
}