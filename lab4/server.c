#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "lab4.h"


static pthread_mutex_t print_lock;
int N, M, end = 0;
char* mq_name;
struct Queue* task_q;
pthread_mutex_t task_m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_cond = PTHREAD_COND_INITIALIZER;


void* receiver(void* arg) {
    int msg_prio = 10;
    mqd_t mqdes, msg[MAX_MSG_SIZE - 1];
    struct mq_attr attr;
    struct timespec t_next, t_sleep = {30, 0};

    mqdes= mq_open (mq_name, O_RDONLY | O_CREAT, 00600, &attr);

    if (mqdes == (mqd_t) -1) {
        perror ( "generator: mq_open" );
        return -1;
    }

    clock_gettime(CLOCK_REALTIME, &t_next);
    TIMESPEC_ADD(t_next, t_sleep);

    while (!end) {
        struct Data data;
        // Receive message from message queue
        mq_timedreceive(mqdes, msg, MAX_MSG_SIZE, msg_prio, &t_next);
        sscanf(msg, "%d %d %s", data.id, data.value, data.name);
        // Lock the queue
        pthread_mutex_lock(&task_m);
        // Add the received data to the queue
        enqueue(task_q, data);
        // Check if there are at least N elements in the queue and the sum of their value variables is at least M
        if (size(task_q) >= N && sumValue(task_q) >= M) {
            // Signal the worker threads
            pthread_cond_broadcast(&task_cond);
        }
        // Unlock the queue
        pthread_mutex_unlock(&task_m);
    }
}

void* worker(void* arg) {
   

    while (1) {
        // Lock the queue
        pthread_mutex_lock(&task_m);
        // Wait for signal from the receiver thread
        pthread_cond_wait(&task_cond, &task_m);
        // Dequeue an element from the queue
        struct Data data = dequeue(task_q);
        // Parse the data
        int id = data.id;
        int value = data.value;
        char name[50];
        strcpy(name, data.name);
        // Print the data
        printf("Id: %d, Value: %d, Name: %s\n", id, value, name);
        // Unlock the queue
        pthread_mutex_unlock(&q->lock);
        // Do some processing with the data
        // ...
        }
}


int main(int argc, char* argv[]) {
    unsigned int i, id, duration, msg_prio;
    char msg[MAX_MSG_SIZE - 1];
    struct shared_id *shd_id;

    if (argc != 3) {
        perror("usage: $./server N M");
        exit(1);
    }

    N = atoi(argv[1]);
    M = atoi(argv[2]);

    mq_name = getenv("SRSV_LAB4");
    if(mq_name == NULL) {
        mq_name = "/lab4sim";
    }

    pthread_t receiver_tid;
    pthread_t worker_tid[N];
    task_q = createQueue();

    if (pthread_create(&receiver_tid, NULL, receiver, NULL)) {
		perror("pthread_create failed");
		exit(1);
	}

    for (i = 0; i < N; i++) {
        if (pthread_create(&worker_tid[i], NULL, worker, NULL)) {
		    perror("pthread_create failed");
		    exit(1);
	    }
    }

    pthread_join(receiver, NULL);
    for (i = 0; i < N; i++) {
        pthread_join(worker_tid[i], NULL);
    }
    
    return 0;
}