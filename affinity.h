#ifdef USE_MACH_AFFINITY
#include <mach/thread_policy.h>
#endif
#ifdef USE_SCHED_AFFINITY
#include <sched.h>
#endif

static void affinity(void)
{

#ifdef USE_SCHED_AFFINITY
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) < 0)
    { perror("sched_setaffinity"); exit(1); }
#endif

#ifdef USE_MACH_AFFINITY
    struct thread_affinity_policy mypolicy;
    mypolicy.affinity_tag = 1;

    if (
        thread_policy_set(
            mach_thread_self(),
            THREAD_AFFINITY_POLICY,
            &mypolicy,
            THREAD_AFFINITY_POLICY_COUNT
            ) != KERN_SUCCESS
        ) {
        perror("thread_policy_set");
        exit(1);
    }
#endif
}
