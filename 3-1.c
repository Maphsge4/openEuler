#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <list>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_mutex1 = PTHREAD_MUTEX_INITIALIZER;

void *threadProc1(void *) {
    printf("threadProc1 enter = %d\n", getpid());
    pthread_mutex_lock(&g_mutex);
    while (1);
    pthread_mutex_unlock(&g_mutex);
    printf("threadProc1 unlock pid = %d\n", getpid());
}

void *threadProc2(void *) {
    printf("threadProc2 enter = %d\n", getpid());
    pthread_mutex_lock(&g_mutex);
    pthread_mutex_unlock(&g_mutex);
    printf("threadProc2 unlock pid = %d\n", getpid());
}

void fun3() {
    printf("fun3 begin\n");
    pthread_mutex_lock(&g_mutex1);
    pthread_mutex_unlock(&g_mutex1);
    printf("fun3 end\n");
}

void fun4() {
    printf("fun4 begin\n");
    pthread_mutex_lock(&g_mutex);
    pthread_mutex_unlock(&g_mutex);
    printf("fun4 end\n");
}

void *threadProc3(void *) {
    printf("threadProc3 enter = %d\n", getpid());
    pthread_mutex_lock(&g_mutex);
    sleep(1);
    fun3();
    pthread_mutex_unlock(&g_mutex);
    printf("threadProc3 unlock pid = %d\n", getpid());
}

void *threadProc4(void *) {
    printf("threadProc4 enter = %d\n", getpid());
    pthread_mutex_lock(&g_mutex1);
    fun4();
    pthread_mutex_unlock(&g_mutex1);
    printf("threadProc4 unlock pid = %d\n", getpid());
}

void fun5() {
    printf("fun5 begin\n");
    pthread_mutex_lock(&g_mutex);
    pthread_mutex_unlock(&g_mutex);
    printf("fun5 end\n");
}

void *threadProc5(void *) {
    printf("threadProc5 enter = %d\n", getpid());
    pthread_mutex_lock(&g_mutex);
    fun5();
    pthread_mutex_unlock(&g_mutex);
    printf("threadProc5 unlock pid = %d\n", getpid());
}

void Thread1() {
    pthread_t tid;
    pthread_create(&tid, NULL, threadProc1, NULL);
    pthread_detach(tid);
}

void Thread2() {
    pthread_t tid;
    pthread_create(&tid, NULL, threadProc2, NULL);
    pthread_detach(tid);
}

void Thread3() {
    pthread_t tid;
    pthread_create(&tid, NULL, threadProc3, NULL);
    pthread_detach(tid);
}

void Thread4() {
    pthread_t tid;
    pthread_create(&tid, NULL, threadProc4, NULL);
    pthread_detach(tid);
}

void Thread5() {
    pthread_t tid;
    pthread_create(&tid, NULL, threadProc5, NULL);
    pthread_detach(tid);
}

//回调接口卡主导致其他线程等锁
void test1() {
    printf("test1 begin\n");
    Thread1();
    Thread2();
    printf("test1 end\n");
}

//两个锁交叉使用导致死锁
void test2() {
    Thread3();
    Thread4();
}

//同一锁被一个线程锁多次
void test3() {
    Thread5();
}

int main(int argc, char *argv[]) {
//    test1();
    //test2();
    test3();
    while (1) {
        sleep(1);
    }
    return 0;
}