#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>

#include "lab3.h"

#define PRIORITY_MAX  6
#define LAB3RT 0

static struct input input[] = {
	{.id = 1, .T = {1, 0}, .t0 = {0, 100000000}},

	{.id = 2, .T = {1, 0}, .t0 = {0, 300000000}},

	{.id = 3, .T = {2, 0}, .t0 = {0, 500000000}},

	{.id = 4, .T = {2, 0}, .t0 = {0, 700000000}},

	{.id = 5, .T = {5, 0}, .t0 = {0, 800000000}},

	{.id = 6, .T = {5, 0}, .t0 = {1, 100000000}},

	{.id = 7, .T = {10, 0}, .t0 = {1, 300000000}},

	{.id = 8, .T = {20, 0}, .t0 = {1, 700000000}},

};

#define INPUT_SIZE sizeof(input) / sizeof(struct input)

static int end = 0;
static unsigned long number;
struct timespec t0;
static struct stat stats;
static pthread_mutex_t print_lock;
static pthread_t ctrl_tid[INPUT_SIZE];
static pthread_t sim_tid[INPUT_SIZE];


void consume_10ms() {
	
    for(unsigned long int i = 0; i < number; i++) {
        asm volatile("" ::: "memory");
    }

}


void compute_iter_cnt_for_10ms() {
	struct timespec t1, t2, t_10ms = {0, 10000000};
	int more = 1;
	number = 1000000;
	while (more) {
		clock_gettime(CLOCK_MONOTONIC, &t1);
		consume_10ms(); // consuming  10ms
		clock_gettime(CLOCK_MONOTONIC, &t2);
		TIMESPEC_SUB(t2, t1);
		if (TIMESPEC_GT(t2, t_10ms)) {
			more = 0;		//lasts long enough
		} else {
			number *= 10;	//lasts too short
		}
	}
	number = number * 10 / (t2.tv_sec * 1000 + t2.tv_nsec / 1e6);
	
}


void simulate_Xms(int x) {
	for(int i = 0; i < x/10; i++) {
		consume_10ms();
	}
}


int simulate_processing_time() {
	int i, proc_time_share;
    i = rand() % 100;
    if (i < 50) {
        proc_time_share = 1;
    } else if (i >= 50 && i < 80) {
        proc_time_share = 2;
    } else if (i >= 80 && i < 95) {
        proc_time_share = 4;
    } else {
        proc_time_share = 7;
    }
	return proc_time_share;
}


static void *controller(void* x) {
	struct input* in = x;
	int state = 0;
	long int proc_time_share;
	struct timespec t_next_period;
	t_next_period = in->t0;
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &in->t0, NULL); 	// wait for input to start

	while (!end) {
		t_next_period = in->t_state;
		TIMESPEC_ADD(t_next_period, t0);
		TIMESPEC_ADD(t_next_period, in->T);

		if (state != in->state) {
			state = in->state;
			TIMESPEC_GET_TIME(in->t_reaction);
			proc_time_share = simulate_processing_time();
			proc_time_share = ((in->T.tv_sec * 1000) + (in->T.tv_nsec / 1e6)) * proc_time_share / 10; //convert period to ms and multiply by percentage of period
			PRINT("CTRL%02d: starting to process state %d\n", in->id, state);
			simulate_Xms(proc_time_share);
			PRINT("CTRL%02d: finished processing state: %d\n", in->id, state);
			in->reply = state;
			TIMESPEC_GET_TIME(in->t_reply);
		}
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_next_period, NULL);
	}
	return NULL;
} 


static void *input_simulator(void* x) {
	struct input* in = x;
	struct timespec t_activation = in->t0, t_activation_h, t_now, msec = {0, 10000000};
	
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_activation, NULL);
	clock_gettime(CLOCK_MONOTONIC, &t_activation);
	t_activation_h = t_activation;
	TIMESPEC_ADD(t_activation_h, in->T); // start of the next period


	while (!end) {
		in->state = rand() % 900 + 100;
		clock_gettime(CLOCK_MONOTONIC, &in->t_state);
		TIMESPEC_SUB(in->t_state, t0);
		in->stat.runs++;
		PRINT("INPUT%02d: state changed to %d\n", in->id, in->state);

		clock_gettime(CLOCK_MONOTONIC, &t_now);
		while ((in->state != in->reply ) && TIMESPEC_GT(t_activation_h, t_now)) { //wait for reply or end of period
			clock_nanosleep(CLOCK_MONOTONIC, 0, &msec, NULL);
			clock_gettime(CLOCK_MONOTONIC, &t_now);
		}


		if (in->state == in->reply) {
			struct timespec wait, reaction;
			wait = in->t_reply;
			TIMESPEC_SUB(wait, in->t_state);	// time took for controller to reply
			TIMESPEC_ADD(in->stat.t_sum_reply, wait);
			if (TIMESPEC_GT(wait, in->stat.t_max_reply)) {
				in->stat.t_max_reply = wait;
			}
			reaction = in->t_reaction;
			TIMESPEC_SUB(reaction, in->t_state); // time too for controller to react to new input
			TIMESPEC_ADD(in->stat.t_sum_reaction, reaction);
			PRINT("INPUT%02d: reply received in %03ld.%06lds ;; controller reaction in %03ld.%06lds\n", in->id,
			 wait.tv_sec/100, wait.tv_nsec/1000, reaction.tv_sec/100, reaction.tv_nsec/1000);
		}
		else {
			PRINT("INPUT%02d: no reply\n", in->id);
			in->stat.overruns++;
		}
		TIMESPEC_ADD(t_activation, in->T);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_activation, NULL); // wait until next period
		t_activation_h = t_activation;
		TIMESPEC_ADD(t_activation_h, in->T);
	}

	return NULL;
	
}


void interrupt_handler(int sig) {
	unsigned int i;
	pthread_mutex_lock(&print_lock);
	if (end) {
		pthread_mutex_unlock(&print_lock);
		return;
	}
	end = 1;
	pthread_mutex_unlock(&print_lock);

	PRINT("Signal %d caught, termination...\n", sig);
	for(i = 0; i < INPUT_SIZE; i++) {
		if (pthread_self() != ctrl_tid[i]) {
			pthread_kill(ctrl_tid[i], SIGTERM);
		}
		if (pthread_self() != sim_tid[i]) {
			pthread_kill(sim_tid[i], SIGTERM);
		}
	}

}


void initialize() {
	unsigned i;
	struct timespec t_zero = {0, 0};

	compute_iter_cnt_for_10ms();

	clock_gettime(CLOCK_MONOTONIC, &t0);
	for (i = 0; i < INPUT_SIZE; i++) {
		TIMESPEC_ADD(input[i].t0, t0);
		input[i].state = 0;
		input[i].reply = 0;
		input[i].t_reaction = t_zero;
		input[i].t_reply = t_zero;
		input[i].t_state = t_zero;
		input[i].stat.runs = 0;
		input[i].stat.overruns = 0;
		input[i].stat.t_max_reply = t_zero;
		input[i].stat.t_sum_reaction = t_zero;
		input[i].stat.t_sum_reply = t_zero;
	}
	
	stats.runs = 0;
	stats.overruns = 0;
	stats.t_max_reply = t_zero;
	stats.t_sum_reaction = t_zero;
	stats.t_sum_reply = t_zero;

	srand(t0.tv_sec);
	srand48(t0.tv_nsec);

	struct sigaction act;
	act.sa_handler = interrupt_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(SIGINT, &act, NULL) || sigaction(SIGTERM, &act, NULL)) {
		perror("Sigaction error!");
		exit(1);
	}

}


void print_stats() {
	unsigned int i;
	double avg;

	for (i = 0; i < INPUT_SIZE; i++) {
		PRINT("\n");
		PRINT("\t INPUT%02d\n", input[i].id);
		PRINT("\truns: %lu\n", input[i].stat.runs);
		PRINT("\toverruns: %ld\n", input[i].stat.overruns);
		avg = input[i].stat.t_sum_reaction.tv_sec * 1e9 + input[i].stat.t_sum_reaction.tv_nsec;
		if (input[i].stat.runs - input[i].stat.overruns > 0) {
			avg = avg / (input[i].stat.runs - input[i].stat.overruns) / 1e9;
		}
		PRINT("\tavg reaction time: %.6fs\n", avg);
		PRINT("\tmax reply time: %ld.%06ld\n", input[i].stat.t_max_reply.tv_sec,
			input[i].stat.t_max_reply.tv_nsec / 1000);
		avg = input[i].stat.t_sum_reply.tv_sec * 1e9 + input[i].stat.t_sum_reply.tv_nsec;
		if (input[i].stat.runs - input[i].stat.overruns > 0) {
			avg = avg / (input[i].stat.runs - input[i].stat.overruns) / 1e9;
		}
		PRINT("\tavg reply time: %.6fs\n", avg);

		stats.runs += input[i].stat.runs;
		stats.overruns += input[i].stat.overruns;
		TIMESPEC_ADD(stats.t_sum_reaction, input[i].stat.t_sum_reaction);
		TIMESPEC_ADD(stats.t_sum_reply, input[i].stat.t_sum_reply);
		if (TIMESPEC_GT(input[i].stat.t_max_reply, stats.t_max_reply)) {
			stats.t_max_reply = input[i].stat.t_max_reply;
		}
	}

	PRINT("\n");
	PRINT("\t OVERALL STATS\n");
	PRINT("\truns: %lu\n", stats.runs);
	PRINT("\toverruns: %lu\n", stats.overruns);
	avg = stats.t_sum_reaction.tv_sec * 1e9 + stats.t_sum_reaction.tv_nsec;
	if (stats.runs - stats.overruns > 0) {
		avg = avg / (stats.runs - stats.overruns) / 1e9;
	}
	PRINT("\tavg reaction time: %.6fs\n", avg);
	PRINT("\tmax reply time: %ld.%06lds\n", stats.t_max_reply.tv_sec,
		stats.t_max_reply.tv_nsec / 1000);
	avg = stats.t_sum_reply.tv_sec * 1e9 + stats.t_sum_reply.tv_nsec;
	if (stats.runs - stats.overruns > 0) {
		avg = avg / (stats.runs - stats.overruns) / 1e9;
	}
	PRINT("\tavg reply time: %.6fs\n", avg);
}


int main() {
	unsigned int i;

	initialize();
	
#if LAB3RT

	long  policy;
	pthread_attr_t attr;
	struct sched_param prio;

	policy = SCHED_FIFO;

	pthread_attr_init ( &attr );
	pthread_attr_setinheritsched ( &attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy ( &attr, policy );

#endif

	PRINT("\tINITIALIZATION COMPLETE, STARTING SIMULATION\n");

	for (i = 0; i < INPUT_SIZE; i++) {
#if LAB3RT
		switch (input[i].T.tv_sec) {
			case 1:
				prio.sched_priority = PRIORITY_MAX -1;
				break;
			case 2:
				prio.sched_priority = PRIORITY_MAX - 2;
				break;
			case 5:
				prio.sched_priority = PRIORITY_MAX -3;
				break;
			case 10:
				prio.sched_priority = PRIORITY_MAX -4;
				break;
			case 20:
				prio.sched_priority = PRIORITY_MAX -5;
				break;
			default:
				prio.sched_priority = PRIORITY_MAX / 2;
				break;
		}
		pthread_attr_setschedparam ( &attr, &prio );
		if (pthread_create(&ctrl_tid[i], &attr, controller, (void *) &input[i])) {
			perror("pthread_create failed");
			exit(1);
		}
		prio.sched_priority = PRIORITY_MAX;
		pthread_attr_setschedparam ( &attr, &prio );
		if (pthread_create(&sim_tid[i], &attr, input_simulator, (void *) &input[i])) {
			perror("pthread_create failed");
			exit(1);
		}
#else
		if (pthread_create(&ctrl_tid[i], NULL, controller, (void *) &input[i])) {
			perror("pthread_create failed");
			exit(1);
		}
		if (pthread_create(&sim_tid[i], NULL, input_simulator, (void *) &input[i])) {
			perror("pthread_create failed");
			exit(1);
		}
#endif
	}

	for (i = 0; i < INPUT_SIZE; i++) {
		pthread_join(ctrl_tid[i], NULL);
	}
	for (i = 0; i < INPUT_SIZE; i++) {
		pthread_join(sim_tid[i], NULL);
	}
		
	print_stats();
	PRINT("\tSIMULATION COMPLETE\n");

	return 0;
}