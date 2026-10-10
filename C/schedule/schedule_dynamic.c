#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SZ 24
#define TH_SZ 6
#define MAX_TIMER_SLOTS 64

double ar[SZ];
int work[SZ];

void init() {
    for (int i = 0; i < SZ; i++) {
        ar[i] = 5000000 + (i * 2000000);
    }
}

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

double schedule_dynamic() {

    int i, th, k;
    double res[SZ], sum = 0.0;

    for (i = 0; i < SZ; i++) work[i] = 0;

#pragma omp parallel default(none) private(i, th, k) shared(ar, res, sum, work) num_threads(TH_SZ)
#pragma omp for schedule(dynamic, 4)

    for (i = 0; i < SZ; i++) {
        th = omp_get_thread_num();
        printf("\niteration %d is executed by thread %d", i, th);
        res[i] = 0;
        for (k = 0; k < ar[i]; k++) {
            res[i] += (sin(0.5*k) + cos(0.5*k));
            work[th] += 1;
        }
    }

    for (i = 0; i < SZ; i++) sum += res[i];
    return sum;
}

int main() {

    init();

    toggle_time(0, "Schedule dynamic");

    printf("\n\t**static result = %g", schedule_dynamic());

    toggle_time(0, "Schedule dynamic");

    return 0;
}