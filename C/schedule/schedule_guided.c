#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <mach/mach_time.h>
#include <math.h>

#define SZ 100
#define TH_SZ 16
#define CHUNK_SIZE 5
#define UTIME_LNG 100

double ar[SZ];

void init() {
    int i;
    for (i = 0; i < SZ; i++) {
        ar[i] = (arc4random() % 1000);
        ar[i] *= 100;
        if(ar[i] > 70000) ar[i] *= 100;
        if(ar[i] < 30000) ar[i] /= 10;
    }
    for (i = 0; i < SZ; i++) {
        printf(" %g", ar[i]);
    }
}

double stopwatch_toggle(int idx, const char *str) {

    static uint64_t t1[UTIME_LNG] = {0};
    static uint64_t t2[UTIME_LNG] = {0};
    static uint8_t first[UTIME_LNG] = {0};

    static mach_timebase_info_data_t timebase_info = {0, 0};
    if (timebase_info.denom == 0) {
        mach_timebase_info(&timebase_info);
    }

    if (__builtin_expect(idx >= UTIME_LNG || idx < 0, 0)) {
        fprintf(stderr, "\n[Error] Wrong index %d given to utimeA().\n", idx);
        exit(-1);
    }

    if (first[idx] == 0) {
        t1[idx] = mach_absolute_time();
        first[idx] = 1;
        return 0.0;
    } else {
        t2[idx] = mach_absolute_time();
        first[idx] = 0;
    }
    uint64_t elapsed_mach = t2[idx] - t1[idx];
    double tval = (double)elapsed_mach * timebase_info.numer / timebase_info.denom / 1e9;
    printf("%s %g second.\n", str, tval);

    return tval;
}

double schedule() {

    int i, th, k;
    double res[SZ], sum = 0.0;

    printf("\nresults: static, thread#= %d", TH_SZ);

#pragma omp parallel default(none) private(i, th, k) shared(res, ar) num_threads(TH_SZ)
#pragma omp for schedule(guided, CHUNK_SIZE)
    for (i = 0; i < SZ; i++) {
        th = omp_get_thread_num();
        printf("\niteration %d is executed by thread %d", i, th);
        res[i] = 0;
        for (k = 0; k < ar[i]; k++) res[i] += (sin(.5*k) + cos(.5*k));
    }
    for(i = 0; i < SZ; i++) sum += res[i];
    return sum;
}


int main() {

    init();
    stopwatch_toggle(1, "");
    printf("\nguided result = %g", schedule());

    return 0;
}