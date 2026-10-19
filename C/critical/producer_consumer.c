#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define QUEUE_SIZE 100
#define MAX_TIMER_SLOTS 64
#define true 1
#define false 0

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

typedef struct {
    int data[QUEUE_SIZE];
    int front;
    int rear;
    int count;
} Queue;

void init(Queue * queue) {
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;
}

int get_jobs(int reset) {
    static int ct = 0;
    static int ar[] = {35, 40, 37, 40, 39, 38, 40, 37, 32, 31};
    if(reset) ct = 0;
    else return ar[ct++];
}

void wait_ms(int milliseconds) {
    clock_t start_time = clock();
    long wait_ticks = (long) (milliseconds * CLOCKS_PER_SEC) / 1000;
    while (clock() < start_time + wait_ticks) {
        // Busy wait - consumes CPU but precise
    }
}

int fib(int n) {
    if(n < 2) return n;
    else return fib(n-1)+fib(n-2);
}

void producer(Queue *qpt) {
    int i;
    int rndN, value;
    int add_queue(Queue *, int);
    get_jobs(1);
    int th;
    for(i = 0; i < 10; i++) {
        rndN = (int) rand() % 5000;
        printf("\nwait time is %d msec", rndN);
        wait_ms(rndN);
        value = get_jobs(0);
        printf("\nAdding value is %d", value);
        add_queue(qpt, value);
        th = omp_get_thread_num();
        printf("\t by %d thread", th);
    }
}

void consumer(Queue *qpt) {
    int value, i;
    int th;
    int empty_queue(Queue *);
    int del_queue(Queue *, int *);
    while(!empty_queue(qpt)) {
        del_queue(qpt, &value);
        printf("\n\tremoved value is %d", value);
        th = omp_get_thread_num();
        printf("\t by %d thread, fib(%d) is %d", th, value, fib(value));
    }
}

void producerP(Queue *qpt, int quantity) {
    int i;
    int rndN, value;
    int add_queue(Queue *, int);
    int th = omp_get_thread_num();
    get_jobs(1);
    printf("\nThread is a producer: Thread ID %d", th);
    for(i = 0; i < quantity; i++) {
        rndN = rand() % 5000;
        printf("\nwait time is %d msec", rndN);
        wait_ms(rndN);
        #pragma omp critical(get_jobs)
        value = get_jobs(0);
        printf("\nAdding value is %d", value);
        th = omp_get_thread_num();
        printf("\t by %d thread", th);
        #pragma omp critical (queue)
        add_queue(qpt, value);
    }
}

void consumerP(Queue *qpt) {
    int value;
    int empty_queue(Queue *);
    int del_queue(Queue *, int *);
    int th = omp_get_thread_num();
    int get_out = 0, wait_ct = 0;
    printf("\nThread is a consumer: Thread ID %d", th);
    do {
        do {
            int val;
            #pragma omp critical (queue)
            {
                val = empty_queue(qpt);
                if(! val) {
                    del_queue(qpt, &value);
                    printf("\n\tremoved value is %d", value);
                    th = omp_get_thread_num();
                    printf("\t by %d thread, fib(%d) is %d", th, value, fib(value));
                }
            }
            if(! val) break;
            wait_ms(20);
            wait_ct ++;
            if(wait_ct > 500) {
                get_out = 1;
                break;
            }
        } while(1);
        if(get_out) {
            printf("\n*** due to lack of job, stop consumerP()");
            break;
        }
    } while(1);
}

int empty_queue(Queue *q) {
    return q->count == 0;
}

int full_queue(Queue *q) {
    return q->count == QUEUE_SIZE;
}

int add_queue(Queue *q, int value) {
    if (full_queue(q)) {
        printf("Queue is full! Cannot add %d\n", value);
        return false;
    }

    q->rear = (q->rear + 1) % QUEUE_SIZE;
    q->data[q->rear] = value;
    q->count++;
    return true;
}

int del_queue(Queue *q, int *value) {
    if (empty_queue(q)) {
        printf("Queue is empty! Cannot remove element\n");
        return false;
    }

    *value = q->data[q->front];
    q->front = (q->front + 1) % QUEUE_SIZE;
    q->count--;
    return true;
}

int main() {
    Queue q;
    init(&q);

    printf("\nThe sequence version:");
    producer(&q);
    consumer(&q);
    printf("\nPress key");
    getchar();

    printf("\nThe parallel version:");
    #pragma omp parallel default(none) shared(q)
    #pragma omp sections
    {
        #pragma omp section
        {
            producerP(&q, 10);
        }
        #pragma omp section
        {
            consumerP(&q);
        }
    }
    printf("\nPress key");
    getchar();
    return(0);
}