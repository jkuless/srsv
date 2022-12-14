#pragma once
#include <time.h>
#define WARN 0
#define STOP 1
struct stat {
	unsigned long int regular_runs;
    unsigned long int extended_runs;
	unsigned long int interrupted_runs;
    unsigned long int proc_time_sum;
};

struct input {
	const int id;
    const int type;
    const struct timespec period;
	struct stat stat;
};

#define CALL(ACT,FUNC,...)         \
do {                               \
    if ( FUNC ( __VA_ARGS__ ) ) {  \
        perror ( #FUNC );          \
        if ( ACT == STOP )         \
            exit (1);              \
    }                              \
} while(0)


#define TIMESPEC_ADD(A,B) /* A += B */ \
do {                                   \
    (A).tv_sec += (B).tv_sec;          \
    (A).tv_nsec += (B).tv_nsec;        \
    if ( (A).tv_nsec >= 1000000000 ) { \
        (A).tv_sec++;                  \
        (A).tv_nsec -= 1000000000;     \
    }                                  \
} while (0)

#define TIMESPEC_SUB(A,B) /* A -= B */ \
do {                                   \
    (A).tv_sec -= (B).tv_sec;          \
    (A).tv_nsec -= (B).tv_nsec;        \
    if ( (A).tv_nsec < 0 ) {           \
        (A).tv_sec--;                  \
        (A).tv_nsec += 1000000000;     \
    }                                  \
} while (0)

#define TIMESPEC_GT(A,B) /* A > B? */ \
( (A).tv_sec > (B).tv_sec || ((A).tv_sec == (B).tv_sec && (A).tv_nsec > (B).tv_nsec) )

#define PRINT(format, ...)                           \
do {                                                  \
    struct timespec t_now;                               \
    clock_gettime(CLOCK_MONOTONIC, &t_now);               \
    TIMESPEC_SUB(t_now, t0);                               \
    pthread_mutex_lock(&print_lock);                      \
    printf("[%03ld:%06ld] " format,                        \
        t_now.tv_sec % 100, t_now.tv_nsec/1000, ##__VA_ARGS__); \
    pthread_mutex_unlock(&print_lock);                       \
} while (0)

