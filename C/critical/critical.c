#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define MAX_TIMER_SLOTS 64
#define TH_SZ 4
#define SZ 1000000

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


void execute_critical() {

    int th, i;
    double sum = 0;
    double val = 0;
#pragma omp parallel default(none) num_threads(TH_SZ) private(th, i) shared(sum) shared(val)
    {
        #pragma omp sections lastprivate(val)
        {
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 1", th);
                for(i = 1; i <= SZ; i++) {
                    val = sin(i*1000) * cos(i*1000);
                }
                #pragma omp critical
                sum += val;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 2", th);
                for(i = 1; i <= SZ; i++) {
                    val = sin(i*1000) * cos(i*1000);
                }
                #pragma omp critical
                sum -= val;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 3", th);
                for(i = 1; i <= SZ; i++) {
                    val = sin(i*1000) * cos(i*1000);
                }
                #pragma omp critical
                sum += val;
            }
        }
    }
    printf("\ntest_critical():");
    printf("\nThe sum is %f", sum);

}

void execute_atomic() {

    int th, i;
    double sum = 0;
    double val = 0;
#pragma omp parallel default(none) num_threads(TH_SZ) private(th, i) shared(sum) shared(val)
    {
        #pragma omp sections lastprivate(val)
        {
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 1", th);
                for(i = 1; i <= SZ; i++) {
                    val = sin(i*1000) * cos(i*1000);
                }
                #pragma omp atomic
                sum += val;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 2", th);
                for(i = 1; i <= SZ; i++) {
                    val = sin(i*1000) * cos(i*1000);
                }
                #pragma omp atomic
                sum -= val;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 3", th);
                for(i = 1; i <= SZ; i++) {
                    val = sin(i*1000) * cos(i*1000);
                }
                #pragma omp atomic
                sum += val;
            }
        }
    }
    printf("\nexecute_atomic():");
    printf("\nThe sum is %f", sum);

}

void execute_reduction() {
    int th, i;
    double sum = 0;
    double val = 0;
#pragma omp parallel default(none) num_threads(TH_SZ) private(th, i) reduction(+: sum) shared(val)
    {
        #pragma omp sections lastprivate(val)
        {
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 1", th);
                for(i = 1; i <= SZ; i++) {
                    val = sin(i*1000) * cos(i*1000);
                }
                sum += val;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 2", th);
                for(i = 1; i <= SZ; i++) {
                    val = sin(i*1000) * cos(i*1000);
                }
                sum -= val;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 3", th);
                for(i = 1; i <= SZ; i++) {
                    val = sin(i*1000) * cos(i*1000);
                }
                sum += val;
            }
        }
    }
    printf("\nexecute_reduction():");
    printf("\nThe sum is %f", sum);
}

int main() {

    toggle_time(0, "Time of execute_critical()");

    execute_critical();

    toggle_time(0, "Time of execute_critical()");

    toggle_time(0, "Time of execute_atomic()");

    execute_atomic();

    toggle_time(0, "Time of execute_atomic()");

    toggle_time(0, "Time of execute_reduction()");

    execute_reduction();

    toggle_time(0, "Time of execute_reduction()");

    return 0;
}