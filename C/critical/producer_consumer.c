#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

#define QUEUE_SIZE 100
#define MAX_TIMER_SLOTS 64
#define TOTAL_JOBS 10
#define TH_NUM 8
#define ARRAY_VALUES {35, 40, 37, 40, 39, 38, 40, 37, 32, 31}

typedef struct Queue Queue;

static int ar[] = ARRAY_VALUES;

double toggle_time(int slot_idx, const char *label) {
    static __thread struct timespec start_times[MAX_TIMER_SLOTS];
    static __thread int is_timer_active[MAX_TIMER_SLOTS] = {0};

    if (slot_idx < 0 || slot_idx >= MAX_TIMER_SLOTS) {
        exit(EXIT_FAILURE);
    }

    if (!is_timer_active[slot_idx]) {
        clock_gettime(CLOCK_MONOTONIC, &start_times[slot_idx]);
        is_timer_active[slot_idx] = 1;
        return 0.0;
    }

    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    is_timer_active[slot_idx] = 0;

    double seconds = (double)(end_time.tv_sec - start_times[slot_idx].tv_sec);
    double nanoseconds = (double)(end_time.tv_nsec - start_times[slot_idx].tv_nsec);
    double elapsed_time = seconds + nanoseconds * 1e-9;

    printf("\n[%s] Executed time: %.6f s.\n", label, elapsed_time);

    return elapsed_time;
}

int fib(int n) {
    if(n < 2) return n;
    else return fib(n-1) + fib(n-2);
}

struct Queue {
    int items[QUEUE_SIZE];
    int front;
    int rear;
    bool (*add)(struct Queue *, int);
    bool (*del)(struct Queue *);
    bool (*isFull)(struct Queue *);
    bool (*isEmpty)(struct Queue *);
};

bool isEmpty(Queue *q) {
    return (q->front == q->rear);
}

bool isFull(Queue *q) {
    return (q->rear == QUEUE_SIZE);
}

bool enqueue(Queue *q, int value) {
    if (q->isFull(q)) {
        printf("[enqueue]: Queue is full\n");
        return false;
    }
    q->items[q->rear] = value;
    q->rear++;
    return true;
}

bool dequeue(Queue *q) {
    if (q->isEmpty(q)) {
        printf("\n[dequeue]: Queue is empty\n");
        return false;
    }
    int value = q->items[q->front];
    int th = omp_get_thread_num();
    printf("[dequeue]: Removed value is %d and is executed by THREAD [%d], fib(%d) is %d \n", value, th, value, fib(value));
    q->front++;
    return true;
}

int fetchJob(int *currentIndex, bool isReset) {
    if (isReset) {
        *currentIndex = 0;
        return -1;
    }

    if (*currentIndex >= TOTAL_JOBS) {
        printf("[fetchJob]: No more jobs available!\n");
        return -1;
    }

    return ar[(*currentIndex)++];
}

void sleep(int milliseconds, int max_ms) {
    if (milliseconds < 0) {
        return;
    }

    if (milliseconds > max_ms) {
        milliseconds = max_ms;
    }

    struct timespec req = {
            .tv_sec = milliseconds / 1000,
            .tv_nsec = (milliseconds % 1000) * 1000000L
    };

    while (nanosleep(&req, &req) == -1) {
        if (errno != EINTR) {
            return;
        }
    }
}

#define sleep_default(milliseconds) sleep(milliseconds, 1000)

void produce(Queue *q) {
    int i;
    int th;
    int rndN;
    int value;
    int currentIndex = 0;
    fetchJob(&currentIndex, true);
    unsigned int seed = (unsigned int)(time(NULL) ^ omp_get_thread_num());
    for(i = 0; i < TOTAL_JOBS; i++) {
        rndN = (int) rand_r(&seed) % 5000;
        sleep_default(rndN);
        value = fetchJob(&currentIndex, 0);
        q->add(q, value);
        th = omp_get_thread_num();
        printf("[produce]: Adding value is %d by THREAD [%d]\n", value, th);
    }
}

void consume(Queue *q) {
    while(!q->isEmpty(q)) {
        q->del(q);
    }
}

void init(Queue * queue) {
    queue->front = 0;
    queue->rear = 0;
    queue->isEmpty = isEmpty;
    queue->isFull = isFull;
    queue->add = enqueue;
    queue->del = dequeue;
}

void sequence(Queue q) {
    produce(&q);
    consume(&q);
}

void produceP(Queue *q) {
    int i;
    int rndN;
    int value;
    int currentIndex;
    unsigned int seed = (unsigned int)(time(NULL) ^ omp_get_thread_num());
    fetchJob(&currentIndex, true);
    for(i = 0; i < TOTAL_JOBS; i++) {
        rndN = (int) rand_r(&seed) % 5000;
        sleep_default(rndN);
        #pragma omp critical
        {
            value = fetchJob(&currentIndex, false);
        }
        #pragma omp critical
        {
            q->add(q, value);
        }
        int th = omp_get_thread_num();
        printf("[produceP]: Adding value is %d by THREAD [%d]\n", value, th);
    }
}

void consumeP(Queue *q) {
    int emptyCount = 0;
    do {
        unsigned int seed = (unsigned int)(time(NULL) ^ omp_get_thread_num());
        sleep_default(seed);
        bool isEmpty = false;
        #pragma omp critical
        {
            isEmpty = q->isEmpty(q);
        }
        if (isEmpty) {
            emptyCount++;
        } else {
            #pragma omp critical
            {
                q->del(q);
            }
            emptyCount = 0;
        }
    } while (emptyCount <= 10);
}

void parallel(Queue q) {
    #pragma omp parallel default(none) shared(q) num_threads(TH_NUM)
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                produceP(&q);
            }

            #pragma omp section
            {
                consumeP(&q);
            }
        }
    }
}

void parallelEx(Queue q, bool isNested) {

    if (isNested) {
        omp_set_max_active_levels(2);
    } else {
        omp_set_max_active_levels(1);
    }

    #pragma omp parallel default(none) shared(q) num_threads(TH_NUM)
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                produceP(&q);
            }

            #pragma omp section
            {
                #pragma omp parallel default(none) shared(q) num_threads(TH_NUM)
                {
                    #pragma omp sections
                    {
                        #pragma omp section
                        {
                            consumeP(&q);
                        }
                        #pragma omp section
                        {
                            consumeP(&q);
                        }
                    }
                }
            }
        }
    }
}


int main() {

    Queue q;
    init(&q);

    printf("[main]: *** The sequence version *** \n");
    toggle_time(0, "sequence()");
    sequence(q);
    toggle_time(0, "sequence()");
    printf("[main]: Press to continue parallel version...\n");
    getchar();

    printf("[main]: *** The parallel version *** \n");
    toggle_time(0, "parallel()");
    parallel(q);
    toggle_time(0, "parallel()");
    printf("[main]: Press to continue extra parallel version...\n");
    getchar();

    printf("[main]: *** The parallel extra version *** \n");
    toggle_time(0, "parallelEx()");
    parallelEx(q, true);
    toggle_time(0, "parallelEx()");
    printf("[main]: Finish...\n");


    return(0);
}