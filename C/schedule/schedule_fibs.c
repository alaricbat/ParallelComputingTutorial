#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>

#define SZ 30
#define TH_SZ 6
#define UTIME_LNG 1000
#define CHUNK 5
double ar[SZ];

void init() {
    int current_val = 15;
    for (int i = 0; i < SZ; i++) {
        ar[i] = (double)current_val;

        current_val += 2;

        if (current_val > 44) {
            current_val = 15;
        }
    }
}

void times2(long *mtime)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    *mtime = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

double measure_elapsed_time(int timer_id, const char *label)
{
    static long start_time[1000];
    static long end_time[1000];
    static int running[1000] = {0};

    if (timer_id < 0 || timer_id >= 1000) {
        printf("Invalid timer ID\n");
        exit(EXIT_FAILURE);
    }

    if (!running[timer_id]) {

        // Start timer
        times2(&start_time[timer_id]);
        running[timer_id] = 1;

        return 0.0;
    }

    // Stop timer
    times2(&end_time[timer_id]);
    running[timer_id] = 0;

    double elapsed =
            (double)(end_time[timer_id] - start_time[timer_id]) / 1000.0;

    printf("%s %g seconds.", label, elapsed);

    return elapsed;
}

int fib(int n) {
    if(n < 3) return(n);
    else return fib(n-1) + fib(n-2);
}

double fib_schedule_serial() {
    int i;
    int th;
    double res[SZ];
    double sum = 0.0;
    for (i = 0; i < SZ; i++) {
        th = omp_get_thread_num();
        printf("\nindex %d is executed by thread %d", i, th);
        res[i] = fib((int)ar[i]);
    }
    for(i = 0; i < SZ; i++) sum += res[i];
    return sum;
}

double fib_schedule_static() {
    int i;
    int j;
    int th;
    double sum = 0;
    double res[SZ];
#pragma omp parallel default(none) shared(res, ar) private(i, j, th) num_threads(TH_SZ)
#pragma omp for schedule(static)
    for (i = 0; i < SZ; i++) {
        th = omp_get_thread_num();
        printf("\nindex %d is executed by thread %d", i, th);
        res[i] = fib((int)ar[i]);
    }
    for (i = 0; i < SZ; i++) sum += res[i];
    return sum;
}

double fib_schedule_dynamic() {
    int i;
    int j;
    int th;
    double sum = 0;
    double res[SZ];
#pragma omp parallel default(none) shared(res, ar) private(i, j, th) num_threads(TH_SZ)
#pragma omp for schedule(dynamic)
    for (i = 0; i < SZ; i++) {
        th = omp_get_thread_num();
        printf("\nindex %d is executed by thread %d", i, th);
        res[i] = fib((int)ar[i]);
    }
    for (i = 0; i < SZ; i++) sum += res[i];
    return sum;
}



int main() {

    init();

    measure_elapsed_time(1,"\nSchedule Serial");
    fib_schedule_serial();
    measure_elapsed_time(1,"\nSchedule Serial");

    measure_elapsed_time(1,"\nSchedule Static");
    fib_schedule_static();
    measure_elapsed_time(1,"\nSchedule Static");

    measure_elapsed_time(1,"\nSchedule Dynamic");
    fib_schedule_dynamic();
    measure_elapsed_time(1,"\nSchedule Dynamic");

    return 0;
};