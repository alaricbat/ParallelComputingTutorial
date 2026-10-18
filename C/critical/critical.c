#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_TIMER_SLOTS 64
#define TH_SZ 4
#define SZ 10

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
    long sum = 0;
#pragma omp parallel default(none) num_threads(TH_SZ) private(th, i) shared(sum)
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 1", th);
                for(i = 1; i <= SZ; i++)
                        #pragma omp critical(abc)
                        sum += i;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 2", th);
                for(i = 1; i <= SZ; i++)
                        #pragma omp critical(abc)
                        sum -= i;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 3", th);
                for(i = 1; i <= SZ; i++)
                        #pragma omp critical(abc)
                        sum += i;
            }
        }
    }
    printf("\ntest_critical():");
    printf("\nThe sum is %ld", sum);

}

void execute_atomic() {

    int th, i;
    long sum = 0;
#pragma omp parallel default(none) num_threads(TH_SZ) private(th, i) shared(sum)
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 1", th);
                for(i = 1; i <= SZ; i++)
                        #pragma omp atomic
                        sum += i;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 2", th);
                for(i = 1; i <= SZ; i++)
                        #pragma omp atomic
                        sum -= i;
            }
            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\nThread %d gets into section 3", th);
                for(i = 1; i <= SZ; i++)
                        #pragma omp atomic
                        sum += i;
            }
        }
    }
    printf("\nexecute_atomic():");
    printf("\nThe sum is %ld", sum);

}

int main() {

    toggle_time(0, "Time of execute_critical()");

    execute_critical();

    toggle_time(0, "Time of execute_critical()");

    toggle_time(0, "Time of execute_atomic()");

    execute_atomic();

    toggle_time(0, "Time of execute_atomic()");

    return 0;
}