#ifndef _WAVECAT_PROFILING_H_
#define _WAVECAT_PROFILING_H_

#include <stdio.h>
#include <sys/time.h>

#if defined(__i386__)

static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

#define CONFIG_HAS_TSC_PROFILING
#elif defined(__x86_64__)

static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

#define CONFIG_HAS_TSC_PROFILING
#endif

#if defined(CONFIG_HAS_TSC_PROFILING) && defined(CONFIG_TSC_PROFILING)
#define PROFILING_SAVE_CYCLES(name) \
	unsigned long long name = rdtsc()
#define PROFILING_PRINT_CYCLES_DIFFERENCE(first, second) \
	fprintf(stderr, "[clc profiling] %s - %s = %llu\n", \
		#first, #second, first - second)
#else
#define PROFILING_SAVE_CYCLES(name) do { } while (0)
#define PROFILING_PRINT_CYCLES_DIFFERENCE(first, second) do { } while (0)
#endif

#if defined(CONFIG_PROFILING)
long diff_ms(struct timeval *first, struct timeval *second)
{
	return ((second->tv_sec - first->tv_sec) * 1000 +
		(second->tv_usec - first->tv_usec) / 1000);
}
#define PROFILING_DEFINE_TIMESTAMP(name) \
	struct timeval name
#define PROFILING_SAVE_TIMESTAMP(name) \
	gettimeofday(&name, NULL)
#define PROFILING_PRINT_TIMESTAMP_DIFFERENCE_MS(first, second) \
	fprintf(stderr, "[ts profiling] %s - %s = %ld\n", \
			#first, #second, diff_ms(&first, &second))
#else
#define PROFILING_DEFINE_TIMESTAMP(name) do { } while (0)
#define PROFILING_SAVE_TIMESTAMP(name) do { } while (0)
#define PROFILING_PRINT_TIMESTAMP_DIFFERENCE_MS(first, second) do { } while (0)
#endif

#endif
