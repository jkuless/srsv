#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include "lab2.h"

#define A_CNT 3
#define B_CNT 4
#define C_CNT 10
#define D_CNT 10
#define E_CNT 16

static struct input input[] = {
    { .id = 1, .type = 0, .period = {1, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 2, .type = 0, .period = {1, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 3, .type = 0, .period = {1, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 4, .type = 1, .period = {2, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 5, .type = 1, .period = {2, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 6, .type = 1, .period = {2, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 7, .type = 1, .period = {2, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 8, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 9, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 10, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 11, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 12, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 13, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 14, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 15, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 16, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 17, .type = 2, .period = {5, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 18, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 19, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 20, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 21, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 22, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 23, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 24, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 25, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 26, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 27, .type = 3, .period = {10, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 28, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 29, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 30, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 31, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 32, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 33, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 34, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 35, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 36, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 37, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 38, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 39, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 40, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 41, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 42, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
    { .id = 43, .type = 4, .period = {20, 0}, .stat = {.extended_runs = 0, .interrupted_runs = 0, .regular_runs = 0, .proc_time_sum = 0}},
};

/*  order of new tasks coming into the system within 1 second

    1 2 3 4 5 6 7 8 9 10
    A B B A C - C D E A

    A->0   B->1   C->2  D->3    E->4    - -> -1 (no new task in this period)
*/ 
const int next[] = {0, 1, 1, 0, 2, -1, 2, 3, 4, 0};

/* next task according to its type */
static int ind[] = {0, 0, 0, 0, 0};

/* number of each task type */
const int task_num[] = {A_CNT, B_CNT, C_CNT ,D_CNT ,E_CNT};

/* number of tasks to be run within a period for each type A, ..., E(4 seconds without this type) */
const int max_tip[] = {A_CNT, B_CNT, C_CNT ,D_CNT ,E_CNT + 4};   

/* task pointer to the first task of the given type*/
const int task_p[] = {0, A_CNT, A_CNT + B_CNT, A_CNT + B_CNT + C_CNT, A_CNT + B_CNT + C_CNT + D_CNT};

int task_current = -1;
int work_current = 0;
int period = 0;
/*         = 0 --> no task in processing
           = 1 --> task being processed in the first period
           = 2 --> task being processed in the second consecutive period
*/
int no_overrun = 0;
int end = 0;
int t = 0;  /* number of interrupts in the current second */

static struct timespec t0;  /* time of simulation start*/
static pthread_mutex_t print_lock;


void interrupt_handler() {
    pthread_mutex_lock(&print_lock);
    if (end) {
	    pthread_mutex_unlock(&print_lock);
		return;
	}
	end = 1;
    pthread_mutex_unlock(&print_lock);
    PRINT("\t SIGINT interrupt, starting termination ...\n");
}

int simulate_processing_time() {
    int i, processing_time;
    i = rand() % 100;
    if (i < 25) {
        processing_time = 30;
    } else if (i >= 25 && i < 70) {
        processing_time = 50;
    } else if (i >= 70 && i < 95) {
        processing_time = 80;
    } else {
        processing_time = 120;
    }
    return processing_time;
}

void process_task() {
    if (task_current != -1) {
        if (period == 1 && no_overrun >= 10) {
            PRINT("\tTASK%02d: 2nd period permitted\n", work_current);
            period = 2;
            no_overrun = 0;
            return;
        } else {
            PRINT("\tTASK%02d: PROCESSING INTERRUPTED!\n", work_current);
            input[work_current - 1].stat.interrupted_runs++;
        }
    }

    int my_work = 0;
    int long processing_time = 0;
    int next_task = 1;

    while (next_task) {
        int skip_period = 0;
        next_task = 0;

        /* get next task from the inputs */
        const int task_type = next[t];
        if (task_type != -1) {
            if(ind[task_type] < task_num[task_type]) {
                task_current = task_type;
                work_current = input[task_p[task_type] + ind[task_type]].id;
            } else {
                skip_period = 1;
            }
            ind[task_type] = (ind[task_type] + 1) % max_tip[task_type];
        } else {
            skip_period = 1;
        }
        t = (t + 1) % 10;
        if(skip_period) {
            task_current = -1;
            work_current = 0;
            period = 0;
            return;
        }
        
        my_work = work_current;
        period = 1;
        struct timespec t_5;
        PRINT("\tTASK%02d: start processing\n", my_work);

        processing_time = simulate_processing_time();

        while (my_work == work_current && processing_time > 0 && !end) {
            t_5.tv_sec = 0;
            t_5.tv_nsec = 5 * 1e6;
            clock_nanosleep(CLOCK_MONOTONIC, 0, &t_5, NULL);
            processing_time -= 5;
            input[my_work - 1].stat.proc_time_sum += 5;
        }
        

        /* task processing is finished or interrupted  */
        if (my_work == work_current && processing_time <= 0 && !end) {
            PRINT("\tTASK%02d: task completed\n", my_work);
            if (period == 1) {
                input[work_current - 1].stat.regular_runs++;
                no_overrun++;
            } else {    
                /* the current task consumed a part of the 2nd period, continue with the next one */
                input[work_current - 1].stat.extended_runs++;
                next_task = 1;
            }
            task_current = -1;
            work_current = 0;
        } else {
            return;
        }
    }
}

void init(timer_t timer, struct sigevent *event, struct itimerspec *period) {
    clock_gettime(CLOCK_MONOTONIC, &t0);
    srand(time(NULL));

    event->sigev_notify = SIGEV_THREAD;
    event->sigev_notify_function = process_task;
    event->sigev_notify_attributes = NULL;

    CALL(STOP, timer_create, CLOCK_MONOTONIC, event, &timer);

    period->it_value.tv_sec = 0;
    period->it_value.tv_nsec = 2 * 1e8;    // first run starts after 0.2s after starting the simulation
    period->it_interval.tv_sec = 0;
    period->it_interval.tv_nsec = 1 * 1e8; // 0.1s between each timer interrupt

    struct sigaction act;
    act.sa_handler = interrupt_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("Error: sigaction failed\n");
        exit(1);
    }
    PRINT("\t SYSTEM INIZIALIZED, STARTING SIMULATION ...\n");
}

void print_stats() {
    long int regular_cnt = 0;
    long int extended_cnt = 0;
    long int interrupted_cnt = 0;
    long int proc_time = 0;
    long int avg_t;

    PRINT("\n");
    PRINT("\t STATS PER TASK:\n");
    for(int i = 0; i < A_CNT + B_CNT + C_CNT + D_CNT + E_CNT; i++) {
        if (input[i].stat.regular_runs + input[i].stat.extended_runs + input[i].stat.interrupted_runs > 0) {
            avg_t = input[i].stat.proc_time_sum / (input[i].stat.regular_runs + input[i].stat.extended_runs + input[i].stat.interrupted_runs);
        } else {
            avg_t = 0;
        }
        PRINT("\t TASK%02d:\n", input[i].id);
        PRINT("\ttype = %d\n", input[i].type);
        PRINT("\tregular runs = %ld\n", input[i].stat.regular_runs);
        PRINT("\textended = %ld\n", input[i].stat.extended_runs);
        PRINT("\tinterrupted runs = %ld\n", input[i].stat.interrupted_runs);
        PRINT("\tavg. processing time = %ldms\n", avg_t);
        PRINT("\n");

        regular_cnt += input[i].stat.regular_runs;
        extended_cnt += input[i].stat.extended_runs;
        interrupted_cnt += input[i].stat.interrupted_runs;
        proc_time += input[i].stat.proc_time_sum;
    }

    avg_t = proc_time / (regular_cnt + extended_cnt + interrupted_cnt);
    PRINT("\t STATS OVERALL\n");
    PRINT("\tregular runs = %ld\n", regular_cnt);
    PRINT("\textended runs= %ld\n", extended_cnt);
    PRINT("\tinterrupted runs = %ld\n", interrupted_cnt);
    PRINT("\tavg. processing time = %ld ms\n", avg_t);
    PRINT("\n");
    PRINT("\t SIMULATION FINISHED\n");
}

int main() {
    timer_t timer;
    struct sigevent event;
    struct itimerspec period;

    init(&timer, &event, &period);

    CALL(STOP, timer_settime, timer, 0, &period, NULL);

    while(!end) {}
    
    print_stats();

    return 0;
}