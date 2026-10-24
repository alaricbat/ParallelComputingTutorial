#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define MAX_TIMER_SLOTS 64
#define TH_SZ 8
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

    printf("[%s] Executed time: %.6f s.\n", label, elapsed_time);

    return elapsed_time;
}

double f(double x) {
    return 2 * log(fabs((1.0 + x) / (1.0 - x)));
}

double trapezoidal(double a, double b) {
    return b - a;
}

void execute() {
    int i;
    double a = 0.0;
    double b = 0.999;
    double h = trapezoidal(a, b) / SZ;

    double sum = f(a) / 2.0;

    for (i = 0; i < SZ; i++) {
        int th = omp_get_thread_num();
//        printf("[execute]: Thread [%d] -> gets into for loop.\n", th);
        double x = a + i * h;
        sum += f(x);
    }

    double final_result = h * (sum + f(b) / 2.0);
    int th = omp_get_thread_num();
    printf("[execute]: Thread [%d] -> final result is {%f}\n", th, final_result);
}

void executeP() {
    int i;
    int th;
    double x;
    double a = 0.0;
    double b = 0.999;
    double h = trapezoidal(a, b) / SZ;

    double sum = f(a) / 2.0;

    #pragma omp parallel default(none) private(i, th, x) shared(a, h, sum) num_threads(TH_SZ)
    #pragma omp for reduction(+:sum)
    for (i = 0; i < SZ; i++) {
        th = omp_get_thread_num();
//        printf("[executeP]: Thread [%d] -> gets into for loop.\n", th);
        x = a + i * h;
        sum += f(x);
    }

    double final_result = h * (sum + f(b) / 2.0);
    th = omp_get_thread_num();
    printf("[executeP]: Thread [%d] -> final result is {%f}\n", th, final_result);
}


int main() {

    toggle_time(0, "Time of execute()");

    execute();

    toggle_time(0, "Time of execute()");

    toggle_time(0, "Time of executeP()");

    executeP();

    toggle_time(0, "Time of executeP()");

    return 0;
}