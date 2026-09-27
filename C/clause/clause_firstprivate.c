#include "omp.h"
#include <stdio.h>
#include <sys/time.h>
#include <stdbool.h>

void get_time_ms(long *mtime) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *mtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

double get_elapsed_time() {

    static long start_time;
    static long end_time;
    static bool is_measuring = false;

    if (is_measuring) {
        get_time_ms(&start_time);
        is_measuring = true;
        return 0.0;
    }

    get_time_ms(&end_time);
    is_measuring = false;

    return (double)(end_time - start_time) / 1000.0;
}


void execute() {

    int sum, th;
    sum = 5;

#pragma omp parallel default(none) firstprivate(sum) private(th) num_threads(4)
    {
        th = omp_get_thread_num();
        sum += 5;
        printf("\nThread %d got sum = %d", th, sum);
    }
    printf("\nafter the parallel block, the sum is %d", sum);
}


int main() {

    printf("\nstart in %lf msec", get_elapsed_time());
    execute();
    printf("\nspent time is %lf msec", get_elapsed_time());

    return 0;
}