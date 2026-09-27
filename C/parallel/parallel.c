#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <math.h>


#define VC2005
#ifdef VC2005
/* save time, in milli second unit, into ltime */
void times2(long *mtime)
{
	__time64_t ltime;
	struct __timeb64 tstruct;
	_time64(&ltime);
	_ftime64_s(&tstruct);
	*mtime = ltime * 1000 + tstruct.millitm;
}
#else
/* save time, in milli second unit, into ltime */
void times2(long *mtime)
{
	time(mtime);
}
#endif

double utime() {
	static long t1, t2;
	static int first = 1;
	if(first) {
		times2(&t1);
		first = 0;
		return(0.0);
	}
	else {
		times2(&t2);
		first = 1;  // reset this counter
	}
	return((t2-t1)/1000.0);
}


#define UTIME_LNG 1000
/* A function to return the used CPU time */
double utimeA(int idx, char *str) {
	static long t1[UTIME_LNG], t2[UTIME_LNG];
	static int first[UTIME_LNG];
	static int initial = 1;
	double tval;
	int i;
	if (initial) {
		initial = 0;
		for (i = 0; i < UTIME_LNG; i++) first[i] = 1;
	}
	if (idx >= UTIME_LNG) {
		printf("\nWrong in giving index to utimeA().\n");
		exit(-1);
	}
	if (first[idx]) {
		times2(&t1[idx]);
		first[idx] = 0;
		return(0.0);
	}
	else {
		times2(&t2[idx]);
		first[idx] = 1;  // reset this counter
	}
	tval = (t2[idx] - t1[idx]) / 1000.0;
	printf("%s %g second.", str, tval);
	return(tval);
}


/* the testing program follows */
//#define SZ 90000
#define SZ 1000
#define TH_SZ 4
double ar[SZ];

double seq_calculation() {
	int i;
	int j;
	double sumy = 0;
	double res[SZ];
	for (i = 0; i < SZ; i++) {
		res[i] = 0;
		for(j = 0; j < 1000; j++) res[i] += (sin(ar[i]) + cos(ar[i]))*j;
	}
	for (i = 0; i < SZ; i++) {
		sumy += res[i];
	}
	return sumy;
}

double parallel_calculation_for() {
	int i;
	int j;
	double sumy = 0;
	double res[SZ];
#pragma omp parallel for private(i, j) num_threads(TH_SZ)
	for (i = 0; i < SZ; i++) {
		res[i] = 0;
		for(j = 0; j < 1000; j++) res[i] += (sin(ar[i]) + cos(ar[i]))*j;
	}
	for (i = 0; i < SZ; i++) {
		sumy += res[i];
	}
	return sumy;
}

double parallel_calculation_direct() {
	double sumy = 0.0;
	double res[SZ];
	int start;
	int i;
	int j;
	int th, sec, p, k;
	sec = SZ / TH_SZ;
	if(SZ % TH_SZ) sec ++;
#pragma omp parallel num_threads(TH_SZ) firstprivate(sec) private(th, i, start, j, p, k)
	{
		th = omp_get_thread_num();
		start = th * sec;
		for(j = 0; j < sec; j++) {
			p = start + j;
			if(p >= SZ) break;
			res[p] = 0;
			for (k = 0; k < 1000; k++) res[p] += (sin(ar[p]) + cos(ar[p]))*k;
		}
	}
	for (i = 0; i < SZ; i++) {
		sumy += res[i];
	}
	return sumy;
}
set_initial() {
	int i;
	for (i = 0; i < SZ; i++) {
		ar[i] = (rand() % 618) / 100.0;
	}
}

main() {
	set_initial();
	utime();
	printf("\n result of sequential is: %f", seq_calculation());
	printf("\ntime spent is %g second. ", utime());
	utime();
	printf("\n result of using parallel only : %f", parallel_calculation_direct());
	printf("\ntime spent is %g second. ", utime());
	utime();
	printf("\n result of using parallel for: %f", parallel_calculation_for());
	printf("\ntime spent is %g second. ", utime());
	printf("\n ... end ... \n");
	getchar();
}



