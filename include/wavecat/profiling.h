#ifndef _WAVECAT_PROFILING_H_
#define _WAVECAT_PROFILING_H_

#include <stdio.h>

#if defined(__i386__)

static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

#define CONFIG_HAS_PROFILING
#elif defined(__x86_64__)

static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

#define CONFIG_HAS_PROFILING
#endif

#if defined(CONFIG_HAS_PROFILING) && defined(CONFIG_PROFILING)
#define PROFILING_SAVE_CYCLES(name) \
	unsigned long long name = rdtsc()
#define PROFILING_PRINT_DIFFERENCE(first, second) \
	fprintf(stderr, "[profiling] %s - %s = %llu\n", \
		#first, #second, first - second)
#else
#define PROFILING_SAVE_CYCLES(name) do { } while (0)
#define PROFILING_PRINT_DIFFERENCE(first, second) do { } while (0)
#endif

#endif
