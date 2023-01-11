#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#include "lab3.h"

static struct input input[] = {
	{.id = 1, .T = {1, 0}, .t0 = {0, 1000000000}},

	{.id = 2, .T = {1, 0}, .t0 = {0, 3000000000}},

	{.id = 3, .T = {2, 0}, .t0 = {0, 5000000000}},

	{.id = 4, .T = {2, 0}, .t0 = {0, 7000000000}},

	{.id = 5, .T = {5, 0}, .t0 = {0, 8000000000}},

	{.id = 6, .T = {5, 0}, .t0 = {1, 1000000000}},

	{.id = 7, .T = {10, 0}, .t0 = {1, 3000000000}},

	{.id = 8, .T = {20, 0}, .t0 = {1, 7000000000}},

};

#define INPUT_SIZE sizeof(input) / sizeof(struct input)

static int end = 0;
static int number;
static struct timespec t0;
static struct stat stats;
static pthread_mutex_t print_lock;
static pthread_t ctrl_tid[INPUT_SIZE];
static pthread_t sim_tid[INPUT_SIZE];


void interrupt_handler(int sig) {
	unsigned int i;
	pthread_mutex_lock(&print_lock);
	if (end) {
		pthread_mutex_unlock(&print_lock);
		return;
	}

	end = 1;
	pthread_mutex_unlock(&print_lock);
	PRINT("Sigint %d caught, termination...\n", sig);
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

	struct sigaction act;
	act.sa_handler = interrupt_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(SIGINT, &act, NULL) || sigaction(SIGTERM, &act, NULL)) {
		perror("Sigaction error!");
		exit(1);
	}

	srand(t0.tv_sec);
	srand48(t0.tv_nsec);

}


void consume_10ms() {
    for(int i = 0; i < number; i++) {
        asm volatile("" ::: "memory");
    }
}


void compute_iter_cnt_for_10ms() {
	struct timespec t1, t_10ms = {0, 10000000};
	int more = 1;
	number = 1000000;
	while (more) {
		TIMESPEC_GET_TIME(t0);
		consume_10ms(); // consuming  10ms
		TIMESPEC_GET_TIME(t1);
		TIMESPEC_SUB(t1, t0);
		if (TIMESPEC_GT(t1, t_10ms)) {
			more = 0;		//lasts long enough
		} else {
			number *= 10;	//lasts too short
		}
	}
	if (t1.tv_sec > 0) {
		number = number * 10 / (t1.tv_sec * 1000 + t1.tv_nsec / 1e6); //number = number * 10 / (t1-t0);
	}
}


void simulate_Xms(int x) {
	for(int i = 0; i < x/10; i++) {
		consume_10ms();
	}
}

long int simulate_processing_time() {
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

//Trajanje obrade simulirati slično kao u prethodnim labosima, ali sada s opisanim radnim čekanjem. 
//Trajanja neka sada budu u postotku periode a ne u ms. Npr. neka u ~50% slučajeva trajanje obrade traje 10% periode,
// u 30% slučajeva 20% periode, u 15% slučajeva 40% periode te u 5% slučajeva 70% periode. Ulaze i njihova svojstva ručno definirati (ne ih generirati na slučajan način). 


void *controller(void* x) {
	struct input* in = x;
	int state;
	long int proc_time_share;
	struct timespec t_now, t_next_period;
	//pričekaj početak rada ulaza i //"prva pojava"
	clock_nanosleep(CLOCK_MONOTONIC, 0, &in->t0, NULL);
	while (!end) {
		//obradi zadatak x
		//čekaj početak iduće periode
		TIMESPEC_GET_TIME(t_next_period);
		TIMESPEC_ADD(t_next_period, in->T);
		state = in->state;
		TIMESPEC_GET_TIME(in->t_reaction);
		PRINT("CTRL%02d: about to process input state %d\n", in->id, state);
		proc_time_share = simulate_processing_time();
		proc_time_share = ((in->T.tv_sec * 1000) + (in->T.tv_nsec / 1e6)) * proc_time_share / 10; //convert period to ms and multiply by percentage of period
		simulate_Xms(proc_time_share);
		PRINT("CTRL%02d: processing of state: %d finished\n", in->id, state);
		in->reply = state;
		TIMESPEC_GET_TIME(in->t_reply);

		TIMESPEC_GET_TIME(t_now);
		while (TIMESPEC_GT(t_next_period, t_now)) {
			TIMESPEC_GET_TIME(t_now);
		}
		/*ulaz[i].t_reakcija = dohvati_sat()
				ispiši(dohvati_sat() + "upravljač, započinjem obradu zadatka" + i)
				spavaj(odredi_vrijeme_obrade())
				ispiši(dohvati_sat() + "upravljač, obrada gotova za zadatak" + i)
				ulaz[i].odgovor = ulaz[i].stanje
				ulaz[i].t_odgovora = dohvati_sat()
		*/
	}

	return NULL;
} 


void *input_simulator(void* x) {
	struct input* in = x;
	struct timespec t_activation = in->t0, t_now, msec = {0, 10000000};
	
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_activation, NULL);
	clock_gettime(CLOCK_MONOTONIC, &t_activation);
	TIMESPEC_SUB(t_activation, t0); // start of periodic changes for this input
	TIMESPEC_ADD(t_activation, in->T); // start of the next period

	while (!end) {
		in->state = rand() % 900 + 100;
		clock_gettime(CLOCK_MONOTONIC, &in->t_state);
		TIMESPEC_SUB(in->t_state, t0);
		in->stat.runs++;
		PRINT("INPUT%02d: state changed to %d\n", in->id, in->state);


		//wait for reply or end of period

		TIMESPEC_GET_TIME(t_now);
		while ((in->state != in->reply ) || TIMESPEC_GT(t_activation, t_now)) {
			clock_nanosleep(CLOCK_MONOTONIC, 0, &msec, NULL);
			TIMESPEC_GET_TIME(t_now);
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
			TIMESPEC_ADD(in->stat.t_sum_reply, reaction);
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
		//odgodi_do(t_akt)
	}

	return NULL;
	//ulaz[i].t_odgovor_avg = ulaz[i].t_odgovor_avg / (ulaz[i].br_promjena - ulaz[i].br_prekasno)
	//ulaz[i].t_reakcija_avg = ulaz[i].t_reakcija_avg / (ulaz[i].br_promjena - ulaz[i].br_prekasno)
	//ispiši("zadatak" + i + "statistika" + ...)
	//br_promjena, br_prekasno, t_max, t_odgovor_avg, t_reakcija_avg
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

	PRINT("INITIALIZATION COMPLETE, STARTING SIMULATION\n");

	for (i = 0; i < INPUT_SIZE; i++) {
		if (pthread_create(&ctrl_tid[i], NULL, controller, (void *) &input[i - 1])) {
			perror("pthread_create failed");
			exit(1);
		}
	}
	for (i = 0; i < INPUT_SIZE; i++) {
		if (pthread_create(&sim_tid[i], NULL, input_simulator, (void *) &input[i - 1])) {
			perror("pthread_create failed");
			exit(1);
		}
	}

	PRINT("Controller and simulator threads created\n");

	for (i = 0; i <= INPUT_SIZE; i++) {
		pthread_join(ctrl_tid[i], NULL);
		pthread_join(sim_tid[i], NULL);
	}
		
	print_stats();

	PRINT("SIMULATION COMPLETED\n");

	return 0;
}