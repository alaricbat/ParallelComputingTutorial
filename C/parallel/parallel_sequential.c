#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define SZ 1000000
#define TH_SZ 4

double ar[SZ];

double seq_calculation(void)
{
    double sumy = 0.0;
    double res[SZ];

    for (int i = 0; i < SZ; i++) {
        res[i] = 0.0;
        for (int j = 0; j < 1000; j++) {
            res[i] += (sin(ar[i]) + cos(ar[i])) * j;
        }
    }

    for (int i = 0; i < SZ; i++) {
        sumy += res[i];
    }

    return sumy;
}

/*
 * Initialize input data
 */
void set_initial(void)
{
    for (int i = 0; i < SZ; i++) {
        ar[i] = (arc4random() % 618) / 100.0;
    }
}

double measure_sequential(void)
{
    printf("\n[measure_sequential][IN]:");
    double start = omp_get_wtime();

    double result = seq_calculation();

    double end = omp_get_wtime();

    printf("\n[measure_sequential]: Result of sequential : %f", result);
    printf("\n[measure_sequential]: Time spent : %f second", end - start);

    printf("\n[measure_sequential][OUT]:");
    return result;
}

int main(void)
{
    set_initial();

    printf("========================================");
    printf("\nOpenMP Parallel Computing Test");
    printf("\nThreads: %d", TH_SZ);
    printf("\nArray size: %d", SZ);
    printf("\n========================================\n");

    measure_sequential();

    return 0;
}