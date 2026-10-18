#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

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


void execute(bool is_nested) {

    int th;
    int sum;
    int final_sum;

    if (is_nested) omp_set_max_active_levels(2);
    else omp_set_max_active_levels(1);

#pragma omp parallel default(none) private(th, sum, final_sum) shared(is_nested) num_threads(TH_SZ)
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                double val = 0.0;
                th = omp_get_thread_num();
                printf("\n[Section 1]: Thread %d just get into the 1st section.", th);
                for(int i = 0; i < 1000; i++) val += sin(i*2*3.14159/100);
            }

            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\n[Section 2]: Thread %d just get into the 2nd section.", th);

                #pragma omp parallel for default(none) private(th, final_sum) shared(is_nested) reduction(+: sum) num_threads(TH_SZ)
                for (int i = 0; i < TH_SZ; i++) {
                    th = omp_get_thread_num();
                    printf("\n\t[Section 2]: Inner for loop, Thread %d is running.", th);
                    sum = (int)fabs((sin(i*20) * 10));
                    printf("\n\t\t=>[Section 2]: Local sum = %d", sum);
                    if (!is_nested) {
                        #pragma omp atomic
                        final_sum += sum;
                    }
                }
                if (is_nested) final_sum = sum;
                printf("\n[Section 2]: Value of total sum = %d", final_sum);
            }

            #pragma omp section
            {
                th = omp_get_thread_num();
                printf("\n[Section 3]: Thread %d just get into the 3rd section.", th);

                #pragma omp parallel sections default(none) num_threads(TH_SZ) private(th)
                {

                    #pragma omp section
                    {
                        printf("\n[Section 3-1] Inner Thread ID: %d, "
                               "Outer Thread ID: %d\n",
                               omp_get_thread_num(),
                               omp_get_ancestor_thread_num(1));
                    }

                    #pragma omp section
                    {
                        printf("\n[Section 3-2] Inner Thread ID: %d, "
                               "Outer Thread ID: %d",
                               omp_get_thread_num(),
                               omp_get_ancestor_thread_num(1));
                    }
                }
            }
        }
        th = omp_get_thread_num();
        printf("\n=> [Sections]: Thread %d goes to the bottom.", th);

    }

}



int main() {

    toggle_time(0, "Time of execute()");

    execute(true);

    toggle_time(0, "Time of execute()");

    return 0;
}