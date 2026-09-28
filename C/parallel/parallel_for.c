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
    double *res = malloc(SZ * sizeof(double));

    if (res == NULL) {
        printf("Failed to allocate memory\n");
        return 0.0;
    }

#pragma omp parallel for default(none) \
                    shared(res, ar) \
                    num_threads(TH_SZ)
    for (int j = 0; j < SZ; j++) {

        int th = omp_get_thread_num();

        if (j % 250000 == 0) {
            printf("Thread %d starts j = %d\n", th, j);
        }

        res[j] = 0.0;

        for (int k = 0; k < 1000; k++) {
            res[j] += (sin(ar[j]) + cos(ar[j])) * k;
        }
    }

    for (int i = 0; i < SZ; i++) {
        sumy += res[i];
    }

    free(res);

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