#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_TIMER_SLOTS 64
#define TH_SZ 4
#define SZ 24

int first_ar[SZ];
int second_ar[SZ];

int first_res[SZ];
int second_res[SZ];

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

    printf("[%s] Executed time: %.6f s.\n", label, elapsed_time);

    return elapsed_time;
}

void init() {
    for (int i = 0; i < SZ; i++) {
        first_ar[i] = 18 + i;
        second_ar[i] = 18 + i;
    }
}

int fib(int n) {
    if(n < 3) return(n);
    else return fib(n-1) + fib(n-2);
}

void execute() {
    int i;
    int th;
#pragma omp parallel default(none) private(i, th) shared(first_ar, second_ar, first_res, second_res) num_threads(TH_SZ)
#pragma omp for schedule(dynamic) nowait
    for (i = 0; i < SZ; i++) {
        first_res[i] = fib(first_ar[i]);
    }
    th = omp_get_thread_num();
    printf("\t thread %d out of the loop.", th);
#pragma omp for schedule(dynamic)
    for (i = 0; i < SZ; i++) {
        second_res[i] = fib(second_ar[i]);
    }
}


int main() {

    init();

    toggle_time(0, "Time of execute()");

    execute();

    toggle_time(0, "Time of execute()");

    return 0;
}