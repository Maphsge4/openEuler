#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<errno.h>
#include<string.h>

#define LIMIT 1000

struct data {
    int n;
    struct data *next;
};

pthread_cond_t condv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
struct data *phead = NULL;

void producer(void *arg) {
    printf("producer thread running.\n");
    int count = 0;
    for (;;) {
        int n = rand() % 100;
        struct data *nd = (struct data *) malloc(sizeof(struct data));
        nd->n = n;

        pthread_mutex_lock(&mlock);
        struct data *tmp = phead;
        phead = nd;
        nd->next = tmp;
        pthread_mutex_unlock(&mlock);
        pthread_cond_signal(&condv);

        count += n;

        if (count > LIMIT) {
            break;
        }
        sleep(rand() % 5);
    }
    printf("producer count=%d\n", count);
}

void consumer(void *arg) {
    printf("consumer thread running.\n");
    int count = 0;
    for (;;) {
        pthread_mutex_lock(&mlock);
        if (NULL == phead) {
            pthread_cond_wait(&condv, &mlock);
        } else {
            while (phead != NULL) {
                count += phead->n;
                struct data *tmp = phead;
                phead = phead->next;
                free(tmp);
            }
        }
        pthread_mutex_unlock(&mlock);
        if (count > LIMIT)
            break;
    }
    printf("consumer count=%d\n", count);
}

int main() {
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, (void *) producer, NULL);
    pthread_create(&tid2, NULL, (void *) consumer, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    return 0;
}