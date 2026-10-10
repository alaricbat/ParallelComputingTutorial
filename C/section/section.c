#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TIMER_SLOTS 64
#define TH_SZ 4

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

int fib(int n) {
    if (n < 3) return n;
    return fib(n - 1) + fib(n - 2);
}

void sections() {

    int th;

#pragma omp parallel default(none) private(th) shared(stdin) num_threads(TH_SZ)
    {
#pragma omp sections
    {

#pragma omp section
    {
        th = omp_get_thread_num();
        printf("[Thread %d] Running Task A: Diary Input\n", th);
        char diary[256];
        if (fgets(diary, sizeof(diary), stdin) != NULL) {
            printf("\n[Thread %d - Task A Result]: Your diary entry: %s", th, diary);
        }
    }

#pragma omp section
    {
        th = omp_get_thread_num();
        printf("[Thread %d] Running Task B: Print Triangle\n", th);
        int rows = 5;
        printf("[Thread %d - Task B Result]:\n", th);
        for (int i = 1; i <= rows; i++) {
            for (int j = 1; j <= i; j++) {
                printf("*");
            }
            printf("\n");
        }
    }

#pragma omp section
    {
        th = omp_get_thread_num();
        printf("[Thread %d] Running Task C: Calculate fib(30) ~ fib(41)\n", th);
        for (int n = 30; n <= 41; n++) {
            int result = fib(n);
            printf("[Thread %d - Task C Result]: fib(%d) = %d\n", th, n, result);
        }
    }

    }

        int th_out = omp_get_thread_num();
        printf(">> Thread %d has passed the implicit sections barrier.\n", th_out);

    }

}


int main() {

    toggle_time(0, "Execute section()");
    sections();
    toggle_time(0, "Execute section()");

    return 0;
}