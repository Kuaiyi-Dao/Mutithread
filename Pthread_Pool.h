#ifndef _PTHREAD_POOL__H
#define _PTHREAD_POOL__H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

typedef struct Task
{
	void (*func)(void *arg);
	void *arg;
	struct Task * prev;
	struct Task * next;
}Task;


typedef struct Workers{
	pthread_t pthreadId;
	int stop;
	struct Workers* prev;
	struct Workers* next;
}workers;

static void *ThreadPoolMain(void *arg);

time_t getTime();

class ThreadPool{
public:
	int minNum;
	int maxNum;
	//int busyNum;
	//int liveNum;
	Task* taskQue;
	workers * worker; //工作线程
	pthread_mutex_t mutexPool; //线程池锁
//	pthread_mutex_t mutexbusy;

//	pthread_cond_t isFull; //条件变量
	pthread_cond_t isEmpty;

	//int shutDown; //销毁线程池标记 1销毁 0运行
	ThreadPool(int min,int max);
	int TaskAdd(Task *task);
	int DestroyPool();
	~ThreadPool();
};

typedef struct args{
	workers* worker;
	ThreadPool* pool;
}args;

#endif