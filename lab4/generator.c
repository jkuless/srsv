#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "lab4.h"

#define MAX_MSG_NBR 50
#define MAX_MSG_SIZE 100


int main(int argc, char* argv[]) {
    int N, M, duration;
    mqd_t mq_desc;
    struct mq_attr attr;
    char *msg = NULL;

    srand(time(NULL));

    if (argc > 3) {
        perror("usage: $./generator N M");
        return -1;
    }
    N = atoi(argv[1]);
    M = atoi(argv[2]);

    char* mq_name = getenv("SRSV_LAB4");
    if(mq_name == NULL) {
        mq_name = "lab4sim";
    }
    
    unsigned priority = 50;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG_NBR;
    attr.mq_msgsize = MAX_MSG_SIZE;

    mq_desc = mq_open (mq_name, O_WRONLY | O_CREAT, 00600, &attr);

    if (mq_desc == (mqd_t) -1) {
        perror ( "generator: mq_open" );
        return -1;
    }

    for (int i; i < N; i++) {
        duration = rand() % M + 1;
        sprintf(msg, "%d %d %s-%d", id, duration, mq_name, id);
        if (mq_send(mq_desc, msg, strlen(msg) + 1, priority)) {
            perror("mq_send");
            return -1;
        sleep(1);
    }
    }
    
    printf ( "Poslano: %s [prio=%d]\n", , prioritet );


    return 0;
}