#include <stdio.h>
#include <sys/time.h>
#include <omp.h>

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


void execute_single() {

    bool is_parallel = false;

#pragma omp parallel default(none) \
                    if(is_parallel) \
                    shared(is_parallel) \
                    num_threads(4)
    {
        printf("Thread ID: %d\n", omp_get_thread_num());
    }
}

void execute_parallel() {

    bool is_parallel = true;

#pragma omp parallel default(none) \
                    if (is_parallel) \
                    num_threads(4)
    {
        printf("Thread ID: %d\n", omp_get_thread_num());
    }
}

int main() {

//    execute_single();
    execute_parallel();

    return 0;
}