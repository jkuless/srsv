#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define N 3 // broj ulaza
#define K 3 // konstanta za simuliranje učestalosti prekida
#define END_COND 2 // broj najduljih period

struct input_static {
    time_t period;  //perioda provjere ulaza
    time_t hand_t;  //vrijeme obrade
    time_t inital_t; //vrijeme prve pojave događaja
};
struct input_dynamic {
    int state;  // stanje ulaza
    int res;    // odgovor
    time_t state_change_t;  // renutak zadnje promjene stanja
    time_t res_t;   // trenutak zadnjeg odgovora
};
struct sim_stat_t{
    int change_cnt; // broj promjena stanja
    int problem_cnt;    //broj prekoracenja vremena za odgovor
    time_t res_t_sum;   //zbroj vremena odgovora 
    time_t max_res_t;   //maksimalno vrijeme odgovora
};

int period[] = {1, 2, 5, 10, 20}; // perioda provjere ulaza [s]
int handling_time[] = {30, 50, 80, 120}; // vremena trajanja obrada [ms]
int time_prob[] = {20, 50, 20, 10};   // vjerojatnost trajanja obrade [%]
int res_cnt[N] = {0}; // broj odgovora za svaki ulaz

time_t global_t = 0;
time_t start_t = 0;
struct input_static input_s[N];
struct input_dynamic input_d[N];
struct sim_stat_t sim_stat[N];

bool end = false;

int get_max_period() {
    int i, max_period = period[0];
    for (i = 0; i < N; i++) {
        if (period[i] > max_period) {
            max_period = period[i];
        }
    }
    return max_period;
}

void print_stats(){
    int i, change_cnt_sum = 0, problem_cnt_sum = 0, res_cnt_sum = 0;
    time_t global_max_res_t = 0, res_t_all_sum = 0;
    printf("------------------------ KRAJ SIMULACIJE ------------------------\n");
    printf("STATISTIKA PO ULAZIMA:\n");
    for(i = 0; i < N; i++) {
        change_cnt_sum += sim_stat[i].change_cnt;
        problem_cnt_sum += sim_stat[i].problem_cnt;
        res_t_all_sum += sim_stat[i].res_t_sum;
        res_cnt_sum += res_cnt[i];
        if (sim_stat[i].max_res_t > global_max_res_t) {
            global_max_res_t = sim_stat[i].max_res_t;
        }
        printf("->ULAZ %2d:\n", i + 1);
        printf("Broj promjena stanja: %d\n", sim_stat[i].change_cnt);
        printf("Broj problema: %d\n", sim_stat[i].problem_cnt);
        printf("Prosječno vrijeme odgovora: %.3f s\n", (double)sim_stat[i].res_t_sum / (res_cnt[i] * 1000));
        printf("Maksimalno vrijeme odgovora: %.3f s\n", (double)sim_stat[i].max_res_t / 1000);
        printf("----------------------------------------------------------------\n");
    }
    printf("ZAJEDNIČKA STATISTIKA:\n");
    printf("Broj ulaza: %d\n", N);
    printf("Broj promjena stanja: %d\n", change_cnt_sum);
    printf("Broj problema: %d\n", problem_cnt_sum);
    printf("Prosječno vrijeme odgovora: %.3f s\n", (double)res_t_all_sum / (res_cnt_sum * 1000));
    printf("Maksimalno vrijeme odgovora: %.3f s\n", (double)global_max_res_t / 1000);
    printf("----------------------------------------------------------------\n");
}


int dohvati_vrijeme_obrade() {
    int n = rand() % 10 + 1;
    int indeks;
    if (n <= 2) {
        indeks = 0;
    } else if (n > 2 && n <= 7) {
        indeks = 1;
    } else if (n > 7 && n <=9) {
        indeks = 2;
    } else {
        indeks = 3;
    }
    return indeks;
}

void *upravljac() {
    //start_t = clock()
    int i, state, max_period;
    int prev_state[N] = {0};
    max_period = get_max_period();

    while (global_t < max_period * END_COND) {
        for (i = 0; i < N; i++) {
            state = input_d[i].state;
            if (state != prev_state[i]) {
                nanosleep(&input_s[i].hand_t, NULL);
                input_d[i].res = state;
                input_d[i].res_t = 
                prev_state[i] = state;
                //todo
            }
        }
    }
    end = true;
    print_stats();
}

void *simulator(void *arg) {
    int id = *(int*)arg;
    struct timespec sleep_1 = {0, 1000000};
    struct timespec init_delay = {input_s[id].inital_t, 0};
    time_t extra_delay;
    
    nanosleep(&init_delay, NULL);

    while (!end) {
        input_d[id].res = 0;
        input_d[id].state = rand() % 900 + 100;
        input_d[id].state_change_t = 
        sim_stat[id].change_cnt++;

        while(input_d[id].res == 0) {
            nanosleep(&sleep_1, NULL);
            sleep_1.tv_sec = 0;
            sleep_1.tv_nsec = 1000000;
        }

		//odgovor je stigao
		//izračunaj trajanje od novog stanja do odgovora
		//ako je to trajanje veće od periode
			//povećaj brojač prekasno pristiglih odgovora
        if(1) {
            
        }
        //izračunaj trajanje od novog stanja do odgovora
		//dodaj to vrijeme u sumu (radi kasnijeg proračuna srednjeg vremena)
		//ako je zadnje trajanje veće od najdužeg ažuriraj i to

		//simulirati da promjene mogu doći rijeđe, ovisno o konstanti K (1-5)
		//extra_delay = rand() % K * input_s[id].period; //random daje realan broj, npr. 1.8

		//odgodi do trenutak zadnje promjene + period + dodatna odgoda
        

    }
    	

	
}

int main(int argc, char* argv[]) {
    int i, *thread_id;
    time_t start, t = 0;
    pthread_t ctrl_thread;  // upravljacka dretva
    pthread_t input_thread[N];  // dretve ulaza
    srand(time(NULL));
    thread_id = malloc(N * sizeof(int));
    if (pthread_create(&ctrl_thread, NULL, upravljac, NULL) != 0) {
        errx(1, "Problem sa stvaranjem upravljacke dretve!");
    }

    for (i = 0; i < N; i++) {
        thread_id[i] = i;
        input_s[i].period = period[3 - dohvati_vrijeme_obrade()];
        input_s[i].hand_t = handling_time[dohvati_vrijeme_obrade()];
        input_s[i].inital_t = rand() % input_s[i].period;
        if(pthread_create(&input_thread[i], NULL, simulator, &thread_id[i]) != 0) {
            errx(2, "Problem sa stvaranjem dretve simulatora: %d", i);
        }
    }

    pthread_join(ctrl_thread, NULL);

    for (i = 0; i < N; i++) {
        pthread_join(input_thread[i], NULL);
    }

    return 0;
}