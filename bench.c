#include "common.h"

#define measure() ((void)0)

#ifdef MEASURE_TSC
uint64_t __gt;
#undef measure
#define measure() __gt = cycles();
#endif
#ifdef MEASURE_CLOCK
clock_t __gt;
#undef measure
#define measure() __gt = clock();
#endif
#ifdef MEASURE_GTOD
struct timeval __gtv;
#undef measure
#define measure() gettimeofday(&__gtv, 0);
#endif
#ifdef MEASURE_TIME
time_t __gt;
#undef measure
#define measure() __gt = time(0);
#endif
#if defined(MEASURE_CRT) || defined(MEASURE_CMT) || defined(MEASURE_CPR) || defined(MEASURE_CTH) || defined(MEASURE_CVIRT) \
    || defined(MEASURE_CRTF) || defined(MEASURE_CRTP) || defined(MEASURE_CMTF) || defined(MEASURE_CMTP) || defined(MEASURE_CPROF) \
    || defined(MEASURE_CHR)
struct timespec __gt;
#undef measure
#define measure() clock_gettime(THECLOCK, &__gt);
#ifdef MEASURE_CRT
#define THECLOCK CLOCK_REALTIME
#endif
#ifdef MEASURE_CMT
#define THECLOCK CLOCK_MONOTONIC
#endif
#ifdef MEASURE_CPR
#define THECLOCK CLOCK_PROCESS_CPUTIME_ID
#endif
#ifdef MEASURE_CHR
#define THECLOCK CLOCK_HIGHRES
#endif
#ifdef MEASURE_CTH
#define THECLOCK CLOCK_THREAD_CPUTIME_ID
#endif
#ifdef MEASURE_CRTF
#define THECLOCK CLOCK_REALTIME_FAST
#endif
#ifdef MEASURE_CRTP
#define THECLOCK CLOCK_REALTIME_PRECISE
#endif
#ifdef MEASURE_CMTF
#define THECLOCK CLOCK_MONOTONIC_FAST
#endif
#ifdef MEASURE_CMTP
#define THECLOCK CLOCK_MONOTONIC_PRECISE
#endif
#ifdef MEASURE_CVIRT
#define THECLOCK CLOCK_VIRTUAL
#endif
#ifdef MEASURE_CPROF
#define THECLOCK CLOCK_PROF
#endif
#endif

#ifdef USE_SETJMP
__attribute__((noinline))
void jthere_func(jmp_buf* env)
{
    ++cnt;
    measure();

    longjmp(*env, 1);
}
#endif

#ifdef USE_SWAPCONTEXT
ucontext_t uhere, uthere;
void uthere_func(void)
{
    // first jump for init
    swapcontext(&uthere, &uhere);
    
    while(1)
    {
        ++cnt;
        measure();
        swapcontext(&uthere, &uhere);
    }
} 

#endif

#ifdef USE_PCL
void pthere_func(void)
{
    // first jump for init
    co_resume();

    while(1)
    {
        ++cnt;
        measure();
        co_resume();
    }
}
#endif

int main(void)
{
    int i;

#ifdef USE_PCL
    coroutine_t pthere;
#endif
#ifdef USE_SETJMP
    jmp_buf env;
#endif

    START_BENCHMARK;

    for (i = 0; i < SAMPLES; ++i)
    {

#ifdef USE_SWAPCONTEXT
        if (getcontext(&uhere))
        { perror("getcontext"); return 1; }
        if (getcontext(&uthere))
        { perror("getcontext"); return 1; }
        uthere.uc_link = 0;
        uthere.uc_stack.ss_sp = (void*)tstack;
        uthere.uc_stack.ss_size = sizeof(tstack);
        uthere.uc_stack.ss_flags = 0;
        makecontext(&uthere, uthere_func, 0);
        swapcontext(&uhere, &uthere); // init context
#endif
#ifdef USE_PCL
        pthere = co_create(pthere_func, 0, &tstack, sizeof(tstack));
        if (pthere == 0)
        { perror("co_create"); return 1; }
        co_call(pthere);
#endif

        START_SAMPLE;

        while(!tag) {
            START_OP;
#ifdef USE_SETJMP
            if (setjmp(env) == 0)
                jthere_func(&env);
#endif
#ifdef USE_SWAPCONTEXT
           swapcontext(&uhere, &uthere);
#endif
#ifdef USE_EMPTYWHILE
           ++cnt;
           measure();
#endif
#ifdef USE_PCL
           co_call(pthere);
#endif
           END_OP;
        }
        END_SAMPLE;

#ifdef USE_PCL
        co_delete(pthere);
#endif
        REPORT_SAMPLE;
    }

    return 0;
}
