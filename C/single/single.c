#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_TIMER_SLOTS 64
#define TH_SZ 8
#define SZ 24

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


void execute() {

    int th;
    int res[TH_SZ];
    int sum = 0;

#pragma omp parallel default(none) private(th) shared(res, sum) num_threads(TH_SZ)
    {
        th = omp_get_thread_num();
        res[th] = th;
        printf("\nThread id = %d", th);
        #pragma omp barrier
        #pragma omp single
        {
            for (int i = 0; i < TH_SZ; i++) {
                printf("\nThread id = %d execute in for loop()", th);
                sum += res[i];
            }
        }
        #pragma omp single
        printf("\nThread ID: [%d] => Sum = %d", th, sum);
    }

}



int main() {

    toggle_time(0, "Time of execute()");

    execute();

    toggle_time(0, "Time of execute()");

    return 0;
}