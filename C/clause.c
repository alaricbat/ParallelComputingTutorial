#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

void times2(long *mtime)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *mtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

double utime()
{
    static long t1, t2;
    static int first = 1;

    if (first) {
        times2(&t1);
        first = 0;
        return 0.0;
    } else {
        times2(&t2);
        first = 1;
    }

    return (t2 - t1) / 1000.0;
}

void clause_using_array() {
    int sum, th;
    int res[4];
    int i = 0;
#pragma omp parallel default(private) shared(res) private(th) num_threads(4)
{
        if (omp_get_thread_num() == 0) {
            printf("\the total thread number is %d", omp_get_num_threads());
        }
        th = omp_get_thread_num();
        res[th] = 5;
}
    for (i; i < 4; i++) {
        sum += res[i];
        printf("\nThread %d got sum = %d", i, sum);
    }
}

void clause_test()
{
    int sum, th;
    int goes = 0;

    sum = 0;

    // Pick one of follows to perform the parallel computing
#pragma omp parallel private(th) shared(sum) num_threads(4)
//#pragma omp parallel firstprivate(th) reduction(+:sum) num_threads(4)
    {
        if (omp_get_thread_num() == 0)
            printf("\nthe total thread number is %d",
                   omp_get_num_threads());

//        sum = 0;
        th = omp_get_thread_num();
        sum += 5;

        printf("\nThread %d got sum = %d", th, sum);
    }

    printf("\nafter the parallel block, the sum is %d", sum);
}

int main()
{
    utime();

    clause_using_array();

    printf("\nuse time is %lf msec", utime());
    printf("\n... end ... \n");

    return 0;
}