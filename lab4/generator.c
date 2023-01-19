#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "lab4.h"

int J, K;

int main(int argc, char* argv[]) {
    unsigned int id, duration, msg_prio;
    char msg[MAX_MSG_SIZE - 1];
    mqd_t mqdes;
    struct mq_attr attr;
    struct shared_id *shd_id;

    srand(time(NULL));
    
    if (argc != 3) {
        perror("usage: $./generator J K");
        exit(1);
    }

    J = atoi(argv[1]);
    K = atoi(argv[2]);

    char* mq_name = getenv("SRSV_LAB4");
    if(mq_name == NULL) {
        mq_name = MQ_DEFAULT_NAME;
    }

    /* create shared memory segment for global task id*/
    id = shm_open(mq_name, O_CREAT | O_RDWR, 00600);
	if ( id == -1 /*|| ftruncate ( id, SHARED_MEM_SIZE ) == -1*/) {
		perror("shm_open/ftruncate");
		exit(1);
	}
	shd_id = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, id, 0);
	if (shd_id == (void *) -1) {
		perror ( "mmap" );
		exit(1);
	}
	close(id);
    
    msg_prio = 10;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG_NBR;
    attr.mq_msgsize = MAX_MSG_SIZE;

    mqdes= mq_open (mq_name, O_WRONLY | O_CREAT, 00600, &attr);

    if (mqdes == (mqd_t) -1) {
        perror ( "generator: mq_open" );
        return -1;
    }

    for (int i = 0; i < J; i++) {
        char task_name[20];
        duration = rand() % K + 1;
        int task_id, msg_data[duration], *shd_task;

        pthread_mutex_lock(&shd_id->gen_lock);
        task_id = shd_id->id;
        shd_id->id++;
        pthread_mutex_unlock(&shd_id->gen_lock);
        sprintf(task_name, "%s-%d", mq_name, task_id);
        /* create shared memory segment for task data*/
        id = shm_open(task_name, O_CREAT | O_RDWR, 00600);
	    if ( id == -1 || ftruncate ( id, duration * sizeof(int) ) == -1) {
	    	perror("shm_open/ftruncate");
	    	exit(1);
	    }
	    shd_task = mmap(NULL, duration * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, id, 0);
	    if (shd_task == (void *) -1) {
	    	perror ( "mmap" );
	    	exit(1);
	    }
	    close(id);

        for(int j = 0; j < duration; j++) {
            msg_data[j] = rand() % 800 + 100;
            *(shd_task + j) = msg_data[j];
        }

        sprintf(msg, "%d %d %s-%d", task_id, duration, mq_name, task_id);
        if (mq_send(mqdes, msg, strlen(msg) + 1, msg_prio)) {
            perror("mq_send");
            return -1;
        }
        printf("G: task %s\n");
        sleep(1);
    }

    printf("id %d\n", shd_id->id);
    munmap (shd_id, SHARED_MEM_SIZE );
	shm_unlink (mq_name);
    mq_unlink (mq_name);
    return 0;
}