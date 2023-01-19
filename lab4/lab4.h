#pragma once
#include <time.h>
#define MAX_QUEUE_SIZE 20
#define MAX_MSG_NBR 10
#define MAX_MSG_SIZE 100
#define MQ_DEFAULT_NAME "/lab4sim"
#define SHARED_MEM_SIZE sizeof(struct shared_id)

struct shared_id {
    unsigned int id;
    pthread_mutex_t gen_lock;
};

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

struct Data {
    int id;
    int value;
    char name[50];
};

struct Queue {
    struct Data items[MAX_QUEUE_SIZE];
    int front;
    int rear;
};

struct Queue* createQueue() {
    struct Queue* q = (struct Queue*) malloc(sizeof(struct Queue));
    q->front = -1;
    q->rear = -1;
    return q;
}

int isEmpty(struct Queue* q) {
    if(q->rear == -1) 
        return 1;
    else 
        return 0;
}

int isFull(struct Queue* q) {
    if((q->rear + 1) % MAX_QUEUE_SIZE == q->front) 
        return 1;
    else 
        return 0;
}

void enqueue(struct Queue* q, struct Data data) {
    if(isFull(q)) 
        printf("Queue is full\n");
    else {
        q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
        q->items[q->rear] = data;
        if(q->front == -1) 
            q->front = q->rear;
    }
}

struct Data dequeue(struct Queue* q) {
    struct Data data;
    if(isEmpty(q)) {
        printf("Queue is empty\n");
    }
    else {
        data = q->items[q->front];
        if(q->front == q->rear) {
            q->front = -1;
            q->rear = -1;
        }
        else {
            q->front = (q->front + 1) % MAX_QUEUE_SIZE;
        }
    }
    return data;
}

void printQueue(struct Queue* q) {
    int i;
    if(isEmpty(q)) {
        printf("Queue is empty\n");
    }
    else {
        printf("Queue elements are: \n");
        for(i = q->front; i != q->rear; i = (i + 1) % MAX_QUEUE_SIZE) {
            printf("Id: %d, Value: %d, Name: %s\n", q->items[i].id, q->items[i].value, q->items[i].name);
        }
        printf("Id: %d, Value: %d, Name: %s\n", q->items[i].id, q->items[i].value, q->items[i].name);
    }
}

int sumValue(struct Queue* q) {
    int sum = 0;
    if(!isEmpty(q)) {
        int i;
        for(i = q->front; i != q->rear; i = (i + 1) % MAX_QUEUE_SIZE) {
            sum += q->items[i].value;
        }
        sum += q->items[i].value;
    }
    return sum;
}


int size(struct Queue* q) {
    int size = 0;
    if(!isEmpty(q)) {
        int i;
        for(i = q->front; i != q->rear; i = (i + 1) % MAX_QUEUE_SIZE) {
            size++;
        }
        size++;
    }
    return size;
}


void destroyQueue(struct Queue* q) {
    free(q);
}