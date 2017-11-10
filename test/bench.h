#ifndef __BENCH_HEADER
#define __BENCH_HEADER

#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>

#ifdef BENCH_RUN
#define BENCH_START(name) double bench_##name##_start = bench_get_time()
#define BENCH_STOP(name) printf("Benchmark %s: %f\n", #name,  bench_get_time() - bench_##name##_start);
#define BENCH_CALL(name, t, f, ...) \
    BENCH_START(name); \
    t = f(__VA_ARGS__); \
    BENCH_STOP(name);
#else
#define BENCH_START(name)
#define BENCH_STOP(name)
#define BENCH_CALL(name, t, f, ...)
#endif // BENCH_RUN

static inline double bench_get_time(){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

#endif
