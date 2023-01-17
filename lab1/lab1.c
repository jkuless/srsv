#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>

#include "lab1.h"

static struct input input[] = {
	{.id = 1, .T = {1, 0}, .t0 = {0, 100000000}},
	{.id = 2, .T = {1, 0}, .t0 = {0, 300000000}},
    {.id = 3, .T = {1, 0}, .t0 = {0, 600000000}},
    {.id = 4, .T = {1, 0}, .t0 = {0, 900000000}},
    {.id = 5, .T = {2, 0}, .t0 = {0, 400000000}},
    {.id = 6, .T = {2, 0}, .t0 = {0, 700000000}},
    {.id = 7, .T = {2, 0}, .t0 = {1, 400000000}},
    {.id = 8, .T = {2, 0}, .t0 = {1, 700000000}},
    {.id = 9, .T = {5, 0}, .t0 = {0, 200000000}},
    {.id = 10, .T = {5, 0}, .t0 = {1, 200000000}},
    {.id = 11, .T = {5, 0}, .t0 = {2, 200000000}},
	{.id = 12, .T = {5, 0}, .t0 = {3, 200000000}},
	{.id = 13, .T = {10, 0}, .t0 = {0, 500000000}},
	{.id = 14, .T = {10, 0}, .t0 = {1, 500000000}},
	{.id = 15, .T = {10, 0}, .t0 = {2, 500000000}},
	{.id = 16, .T = {10, 0}, .t0 = {3, 500000000}},
	{.id = 17, .T = {10, 0}, .t0 = {4, 500000000}},
    {.id = 18, .T = {10, 0}, .t0 = {5, 500000000}},
    {.id = 19, .T = {10, 0}, .t0 = {6, 500000000}},
    {.id = 20, .T = {10, 0}, .t0 = {7, 500000000}},
    {.id = 21, .T = {10, 0}, .t0 = {9, 500000000}},
    {.id = 22, .T = {20, 0}, .t0 = {0, 800000000}},
    {.id = 23, .T = {20, 0}, .t0 = {1, 800000000}},
    {.id = 24, .T = {20, 0}, .t0 = {2, 800000000}},
    {.id = 25, .T = {20, 0}, .t0 = {3, 800000000}},
    {.id = 26, .T = {20, 0}, .t0 = {4, 800000000}},
    {.id = 27, .T = {20, 0}, .t0 = {9, 800000000}},
    {.id = 28, .T = {20, 0}, .t0 = {10, 800000000}},
    {.id = 29, .T = {20, 0}, .t0 = {11, 800000000}},
    {.id = 30, .T = {20, 0}, .t0 = {13, 800000000}},
    {.id = 31, .T = {20, 0}, .t0 = {15, 800000000}},
    {.id = 32, .T = {20, 0}, .t0 = {16, 800000000}},
    {.id = 33, .T = {20, 0}, .t0 = {17, 800000000}},
    {.id = 34, .T = {20, 0}, .t0 = {18, 800000000}},
};

#define INPUT_SIZE sizeof(input) / sizeof(struct input)

static int end = 0;
struct timespec t0;
static struct stat stats;
static pthread_mutex_t print_lock;
static pthread_t ctrl_tid;
static pthread_t sim_tid[INPUT_SIZE];

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
		if (pthread_self() != sim_tid[i]) {
			pthread_kill(sim_tid[i], SIGTERM);
		}
	}
    if (pthread_self() != ctrl_tid) {
        pthread_kill(ctrl_tid, SIGTERM);
    }
}


int simulate_processing_time() {
	int i, proc_time;
    i = rand() % 100;
    if (i < 20) {
        proc_time= 30;
    } else if (i >= 20 && i < 70) {
        proc_time = 50;
    } else if (i >= 70 && i < 90) {
        proc_time = 80;
    } else {
        proc_time = 120;
    }
	return proc_time;
}


void initialize() {
	unsigned i;
	struct timespec t_zero = {0, 0};

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


static void *controller(void* x) {
	struct input* in = x;
    unsigned int i, state;
	long int proc_time;
	struct timespec t_now, t_next_period, t_proc_time;
	//clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &in->t0, NULL); 	// wait for input to start

	while (!end) {
        for(i = 0; i < INPUT_SIZE && !end; i++) {
            state = in[i].state;
		    TIMESPEC_GET_TIME(in[i].t_reaction);
		    t_next_period = in[i].t_state;
		    TIMESPEC_ADD(t_next_period, in[i].T);
		    proc_time = simulate_processing_time();
            t_proc_time = {0, proc_time * 1e6};
		    PRINT("CTRL%02d: starting to process state %d\n", in[i].id, state);
		    clock_nanosleep(CLOCK_MONOTONIC, 0, &t_proc_time, NULL);
		    PRINT("CTRL%02d: finished processing state: %d\n", in[i].id, state);
		    in[i].reply = state;
		    TIMESPEC_GET_TIME(in[i].t_reply);
        }
	}

	return NULL;
} 


static void *input_simulator(void* x) {
	struct input* in = x;
	struct timespec t_activation = in->t0, t_activation_h, t_now, msec = {0, 10000000};
	
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_activation, NULL);
	clock_gettime(CLOCK_MONOTONIC, &t_activation);
	//TIMESPEC_SUB(t_activation, t0); // start of periodic changes for this input
	t_activation_h = t_activation;
	TIMESPEC_ADD(t_activation_h, in->T); // start of the next period


	while (!end) {
		in->state = rand() % 900 + 100;
		clock_gettime(CLOCK_MONOTONIC, &in->t_state);
		TIMESPEC_SUB(in->t_state, t0);
		in->stat.runs++;
		PRINT("INPUT%02d: state changed to %d\n", in->id, in->state);

		//wait for reply or end of period

		clock_gettime(CLOCK_MONOTONIC, &t_now);
		while ((in->state != in->reply ) && TIMESPEC_GT(t_activation_h, t_now)) {
			clock_nanosleep(CLOCK_MONOTONIC, 0, &msec, NULL);
			clock_gettime(CLOCK_MONOTONIC, &t_now);
		}

		//dok ulaz[i].stanje != ulaz[i].odgovor ILI t_akt + ulaz[i].T > dohvati_sat()
		//	spavaj(10 ms)

		if (in->state == in->reply) {
			struct timespec wait, reaction;
			//TIMESPEC_GET_TIME(wait);
			wait = in->t_reply;
			TIMESPEC_SUB(wait, in->t_state);
			//cekanje = dohvati_sat() - ulaz[i].t_stanja
			TIMESPEC_ADD(in->stat.t_sum_reply, wait);
			//ulaz[i].t_odgovor_avg += cekanje
			if (TIMESPEC_GT(wait, in->stat.t_max_reply)) {
				in->stat.t_max_reply = wait;
			}
			//ulaz[i].t_max = max(ulaz[i].t_max, cekanje)
			reaction = in->t_reaction;
			TIMESPEC_SUB(reaction, in->t_state);
			//reakcija = ulaz[i].t_reakcija - ulaz[i].t_stanja
			TIMESPEC_ADD(in->stat.t_sum_reaction, reaction);
			//ulaz[i].t_reakcija_avg += reakcija
			PRINT("INPUT%02d: reply received in %03ld.%06lds ;; controller reaction in %03ld.%06lds\n", in->id,
			 wait.tv_sec/100, wait.tv_nsec/1000, reaction.tv_sec/100, reaction.tv_nsec/1000);
			//ispiši(dohvati_sat() + "ulaz" + i + "dobio odgovor za" + cekanje,
			//"reakcija na promjenu za" + reakcija)
		}
		else {
			PRINT("INPUT%02d: no reply\n", in->id);
			//ispiši(dohvati_sat() + "ulaz" + i + "nema odgovora")
			in->stat.overruns++;
			//ulaz[i].br_prekasno++
		}
		TIMESPEC_ADD(t_activation, in->T);
		//t_akt += ulaz[i].T
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_activation, NULL);
		t_activation_h = t_activation;
		TIMESPEC_ADD(t_activation_h, in->T);
		//odgodi_do(t_akt)
	}

	return NULL;
	
}


int main() {
	unsigned int i;

	initialize();

	PRINT("\tINITIALIZATION COMPLETE, STARTING SIMULATION\n");

    if (pthread_create(&ctrl_tid, NULL, controller, (void *) &input)) {
		perror("pthread_create failed");
		exit(1);
	}

	for (i = 0; i < INPUT_SIZE; i++) {
		if (pthread_create(&sim_tid[i], NULL, input_simulator, (void *) &input[i])) {
			perror("pthread_create failed");
			exit(1);
		}
	}

	pthread_join(ctrl_tid, NULL);
	for (i = 0; i < INPUT_SIZE; i++) {
		pthread_join(sim_tid[i], NULL);
	}
		
	print_stats();

    PRINT("\tSIMULATION COMPLETE\n");

	return 0;
}