#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define MAX_TIMER_SLOTS 64
#define TH_SZ 4
#define SZ 100

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

void execute() {
    int j;
    int th;
    double x;
    double arr[101] = {0};
    double list[101] = {0};
    #pragma omp parallel default(none) shared(arr, list) private(j, th, x) num_threads(TH_SZ)
    #pragma omp for ordered
    for (j = 1; j < SZ; j++) {
        th = omp_get_thread_num();
        printf("THREAD [%d] executes item [%d].\n", th, j);
        list[j] = 0;
        for(x = 0; x < 1000.0; x += 0.5) {
            list[j] += cos(x);
        }
        #pragma omp ordered
        {
            printf("THREAD [%d] - item [%d] gets into ordered.\n", th, j);
            arr[j] = arr[j-1] + list[j];
        }
    }
}


int main() {

    toggle_time(0, "Time of execute()");

    execute();

    toggle_time(0, "Time of execute()");

    return 0;
}