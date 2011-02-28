__attribute__((always_inline))
static int64_t cycles(void) {
#if defined(__alpha__)
    uint32_t tsc;
    __asm__ __volatile__("rpcc %0" : "=r"(tsc));
    return tsc;
#elif defined(__i386__) || defined(__x86_64__)
    uint32_t lo, hi;
    /* We cannot use "=A", since this would use %rax on x86_64 */
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return (uint64_t)hi << 32 | lo;
#elif defined(__sparc64__) || defined(__sparc__)
    /* NOTE: this does not work when running in 32-bit ABI on a 64-bit machine */
     uint64_t tsc;
    __asm__ __volatile__("rd %%tick, %0" : "=r"(tsc));
    return tsc;
/* the following might:

uint32_t low, high;
    __asm__ __volatile__("rd %%tick, %%g1\n\tsrlx %%g1,32,%0\n\tsra %%g1,0,%1" : "=r"(high), "=r"(low));
    return (((uint64_t)high) << 32)|low);
*/
#else
#error Cannot read time counter here.
#endif
}
