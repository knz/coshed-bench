#include "common.h"

#if defined(USE_SWAPCONTEXT) || defined(USE_PCL)
static
void there_func(void)
{
} 
#endif


int main(void)
{
    int i;

#ifdef USE_SETJMP
    jmp_buf env;
#endif
#ifdef USE_SWAPCONTEXT
    ucontext_t ctx;
#endif
#ifdef USE_PCL
    coroutine_t co;
#endif

    START_BENCHMARK;
    
    for (i = 0; i < SAMPLES; ++i)
    {
        START_SAMPLE;
        while(!tag) {
            ++cnt;
            START_OP;
#ifdef USE_SETJMP
            if (setjmp(env))
                break;
#endif
#ifdef USE_SWAPCONTEXT
            getcontext(&ctx);
            ctx.uc_link = 0;
            ctx.uc_stack.ss_sp = (void*)tstack;
            ctx.uc_stack.ss_size = sizeof(tstack);
            ctx.uc_stack.ss_flags = 0;
            makecontext(&ctx, there_func, 0);
#endif
#ifdef USE_PCL
            co = co_create(there_func, 0, &tstack, sizeof(tstack));
#endif
            END_OP;
        }

        END_SAMPLE;

        REPORT_SAMPLE;
    }
    
    return 0;
}
