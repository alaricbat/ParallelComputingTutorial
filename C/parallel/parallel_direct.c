#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define SZ 1000000
#define TH_SZ 4

double ar[SZ];

void initial(void) {
    for (int i = 0; i < SZ; i++) {
        ar[i] = (arc4random() % 618) / 100.0;
    }
}


double parallel(void) {

    double sumy = 0.0;
    double res[SZ];
    int sec = SZ / TH_SZ;

#pragma omp parallel default(none) firstprivate(sec) shared(ar, res) num_threads(TH_SZ)
{
    int th = omp_get_thread_num();
    int start = th * sec;

    for (int j = 0; j < sec; j++) {
        int p = start + j;

        if (p >= SZ) {
            break;
        }

        res[p] = 0.0;

        for (int k = 0; k < 1000; k++) {
            res[p] +=
                    (sin(ar[p]) + cos(ar[p])) * k;
        }
    }
}

    for (int i = 0; i < SZ; i++) {
        sumy += res[i];
    }

    return sumy;

}


void measure(void) {

    double start = omp_get_wtime();

    double result = parallel();

    double end = omp_get_wtime();

    printf("\nResult of parallel        : %f", result);
    printf("\nTime spent                : %f second", end - start);

}

int main(void) {

    initial();

    printf("========================================");
    printf("\nOpenMP Parallel Computing Test");
    printf("\nThreads: %d", TH_SZ);
    printf("\nArray size: %d", SZ);
    printf("\n========================================\n");

    measure();

    return 0;
}