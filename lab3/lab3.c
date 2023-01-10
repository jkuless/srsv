#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#include <lab3.h>

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

void initialize() {

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
		number = number * 10 / t1.tv_sec; //number = number * 10 / (t1-t0);
	}
}


void simulate_Xms(int x) {
	for(int i = 0; i < x/10; i++) {
		consume_10ms();
	}
}


void controller(void* x) {
	struct input* in = x;
	//pričekaj početak rada ulaza i //"prva pojava"
	clock_nanosleep(CLOCK_MONOTONIC, 0, &in->t0, NULL);
	while (1) {
		obradi zadatak x
		čekaj početak iduće periode
	}
} 


void input_simulator(void* x)
{
	struct input* in = x;
	struct timespec t_activation = in->t0, t_activation_h, t_now, msec = {0, 10000000};
	
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_activation, NULL);
	clock_gettime(CLOCK_MONOTONIC, &t_activation);
	TIMESPEC_SUB(t_activation, t0); // start of periodic changes for this input
	t_activation_h = t_activation;
	TIMESPEC_ADD(t_activation_h, in->T); // start of the next period

	while (!end) {
		in->state = rand() % 900 + 100;
		clock_gettime(CLOCK_MONOTONIC, &in->t_state);
		TIMESPEC_SUB(in->t_state, t0);
		in->stat.runs++;
		PRINT("INPUT%02d: state changed to %d\n", in->id, in->state);


		//wait for reply or end of period

		TIMESPEC_GET_TIME(t_now);
		while ((in->state != in->reply ) || TIMESPEC_GT(t_activation_h, t_now)) {
			clock_nanosleep(CLOCK_MONOTONIC, 0, &msec, NULL);
		}

		//dok ulaz[i].stanje != ulaz[i].odgovor ILI t_akt + ulaz[i].T > dohvati_sat()
		//	spavaj(10 ms)

		if (in->state != in->reply) {
			struct timespec wait, reaction;
			TIMESPEC_GET_TIME(wait);
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
			PRINT("INPUT%02d: reply received in %03ld.%06lds ;; controller reaction in %03ld.%06lds\n",
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
		clock_nanosleep(CLOCK_MONOTONIC, 0, &t_activation, NULL);
		//odgodi_do(t_akt)
	}
	//na kraju

	//ulaz[i].t_odgovor_avg = ulaz[i].t_odgovor_avg / (ulaz[i].br_promjena - ulaz[i].br_prekasno)
	//ulaz[i].t_reakcija_avg = ulaz[i].t_reakcija_avg / (ulaz[i].br_promjena - ulaz[i].br_prekasno)
	//ispiši("zadatak" + i + "statistika" + ...)
	//br_promjena, br_prekasno, t_max, t_odgovor_avg, t_reakcija_avg
}

void print_stats() {

}

int main() {
	unsigned int i;

	initialize();

	PRINT("INITIALIZATION COMPLETE, STARTING SIMULATION\n");

	for (i = 0; i < INPUT_SIZE; i++)
		if (pthread_create(&ctrl_tid[i], NULL, controller, (void *) &input[i - 1])) {
			perror("pthread_create failed");
			exit(1);
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