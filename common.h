#include <inttypes.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#ifdef USE_SWAPCONTEXT
#include <ucontext.h>
#endif
#ifdef USE_PCL
#include <pcl.h>
#endif

#ifndef INFINITY
#define INFINITY HUGE_VAL
#endif

#include "cycles.h"

char tstack[65536];

static
volatile int tag = 0;
static
volatile double cnt = 0;

static
void sigh(int unused __attribute__((unused)))
{
    tag = 1;
}

#include "affinity.h"

static
void reset(void)
{
    struct itimerval itval;

    tag = 0;
    cnt = 0;

    signal(SIGALRM, sigh);
    itval.it_interval.tv_sec = 0;
    itval.it_interval.tv_usec = 0;
    itval.it_value.tv_sec = 0;
    itval.it_value.tv_usec = 10000;
    setitimer(ITIMER_REAL, &itval, 0);
}

static
void enable_tsc(void)
{
#if defined(__linux__) && defined(PR_SET_TSC) && defined(PR_TSC_ENABLE)
    int e = prctl(PR_SET_TSC, PR_TSC_ENABLE, 0, 0, 0);
    if (e < 0)
    { fprintf(stderr, "prctl: %s\n", strerror(errno)); }
#endif
}

#define START_BENCHMARK                                                 \
    int64_t ts_before, ts_after, ts_early, ts_late;                     \
    double Smin = +INFINITY;                                            \
    struct timeval tv_before, tv_after;                                 \
    printf("%s\n", MESSAGE);                                            \
    affinity();                                                         \
    enable_tsc();                                                       \
    printf("# -- Key --\n"                                              \
           "# round: sampling round\n"                                  \
           "# secs: duration of sample (seconds)\n"                     \
           "# it: number of iterations\n"                               \
           "# cy: total number of CPU cycles for round\n"               \
           "# it/sec: iterations per second\n"                          \
           "# cy/sec: number of cycles per second\n"                    \
           "# cy/it: total number of cycles per iteration\n"            \
           "# mcymin: minimum number of cycles per individual operation (mcy)\n" \
           "# mcymax: maximum mcy\n"                                    \
           "# mcymean: average mcy\n"                                   \
           "# mcystddev: standard deviation of mcy\n"                   \
           "# mcyreldev: mcystddev / mcymean\n"                         \
           "# mcystderr: standard error of mcy\n"                       \
           "# -- --\n"                                                  \
           "round\tsecs\tit\tcy\tit/sec\tcy/sec\tcy/it\t"               \
           "mcymin\tmcymax\tmcymean\tmcystddev\tmcyreldev\tmcystderr\n"); \

#define REPORT_SAMPLE                                                   \
    double sec = (tv_after.tv_sec - tv_before.tv_sec)                   \
        + 1e-6 * (tv_after.tv_usec - tv_before.tv_usec);                \
    unsigned long long cy = ts_late-ts_early;                           \
                                                                        \
    double S = sqrt(Q/(cnt+1.0));                                       \
    if (S == 0.0 || S >= Smin) continue;                                 \
    Smin = S;                                                           \
                                                                        \
    printf("%d\t%.6lf\t%lu\t%llu\t", i, sec, (unsigned long)cnt, cy);   \
    printf("%.6lf\t", (double)cnt/sec);                                 \
    printf("%.6lf\t", (double)cy/sec);                                  \
    printf("%.6lf\t", (double)cy/cnt);                                  \
    printf("%llu\t", (unsigned long long)cymin);                        \
    printf("%llu\t", (unsigned long long)cymax);                        \
    printf("%.6lf\t", A);                                               \
    printf("%.6lf\t", S);                                               \
    printf("%.2lf%%\t", 100*S/A);                                       \
    printf("%.6lf", S/sqrt(cnt));                                       \
    printf("\n");

    
#define START_SAMPLE                 \
    double A = 0.;                   \
    double Q = 0.;                   \
    uint64_t cymin = UINT64_MAX;     \
    uint64_t cymax = 0;              \
    reset();                         \
    gettimeofday(&tv_before, 0);     \
    ts_early = cycles();

#define START_OP \
    ts_before = cycles();

#define END_OP                                    \
    ts_after = cycles();                          \
    /* stats */                                   \
    uint64_t ix = (ts_after - ts_before);         \
    if (ix < cymin) cymin = ix;                   \
    if (ix > cymax) cymax = ix;                   \
    double x = (double)ix;                        \
    double Ap = A;                                \
    double An = Ap + (x - Ap) / cnt;              \
    Q = Q + (x - Ap)*(x - An);                    \
    A = An;

#define END_SAMPLE                              \
    ts_late = cycles();                         \
    gettimeofday(&tv_after, 0);


