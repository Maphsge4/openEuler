#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<errno.h>
#include<string.h>
#include<semaphore.h>

#define  NUM  5

int queue[NUM];
sem_t psem, csem;

void producer(void *arg) {
    int pos = 0;
    int num, count = 0;
    for (int i = 0; i < 12; ++i) {
        num = rand() % 100;
        count += num;
        sem_wait(&psem);
        queue[pos] = num;
        sem_post(&csem);
        printf("producer:  %d\n", num);
        pos = (pos + 1) % NUM;
        sleep(rand() % 2);
    }
    printf("producer  count=%d\n", count);
}

void consumer(void *arg) {
    int pos = 0;
    int num, count = 0;
    for (int i = 0; i < 12; ++i) {
        sem_wait(&csem);
        num = queue[pos];
        sem_post(&psem);
        printf("consumer:  %d\n", num);
        count += num;
        pos = (pos + 1) % NUM;
        sleep(rand() % 3);
    }
    printf("consumer  count=%d\n", count);
}

int main() {
    sem_init(&psem, 0, NUM);
    sem_init(&csem, 0, 0);

    pthread_t tid[2];
    pthread_create(&tid[0], NULL, (void *) producer, NULL);
    pthread_create(&tid[1], NULL, (void *) consumer, NULL);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    sem_destroy(&psem);

    sem_destroy(&csem);


    return 0;

}