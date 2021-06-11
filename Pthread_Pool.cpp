#include "Pthread_Pool.h"

template <typename T>
int Addto(T &Head,T&New){
	New->prev = NULL;
	New->next = Head;
	if (Head!=NULL)Head->prev = New;
	Head = New;
	return 0;
}

template <typename T>
int Remv(T &List,T&Item){
	if (Item->prev != NULL)Item->prev->next = Item->next;
	if (Item->next != NULL)Item->next->prev = Item->prev;
	if (List == Item) List = Item->next;
	Item->prev = Item->next = NULL;
	return 0;
}

ThreadPool::ThreadPool(int min,int max){
	if (min < 1) min = 1;
	if (max < 1) max = 1;
	//if (quCapacity < 1) = quCapacity = 1;
	minNum = min;
	maxNum = max;
	//quCapacity = quCapacity;
	taskQue = NULL;

	isEmpty = PTHREAD_COND_INITIALIZER;
	mutexPool = PTHREAD_MUTEX_INITIALIZER;
	args Arg = {NULL,NULL};
	for (int i = 0; i < maxNum;i++){
		workers *Worker = (workers*)malloc(sizeof(workers));
		if (Worker == NULL){
			perror("malloc fail\n");
		}
		memset(Worker,0,sizeof(workers));
		Worker->stop = 0;
		Arg.worker = Worker;
		Arg.pool = this;
		pthread_create(&(Worker->pthreadId),NULL,ThreadPoolMain,(void*)&Arg);
		//printf("%lld\n",Worker->pthreadId);
		Addto(worker,Worker);
	}
} 

static void *ThreadPoolMain(void *arg){
	args* Args = (args*)arg;
	ThreadPool * This = Args->pool;
	workers * OneWorker = Args->worker;
	while(true){
		pthread_mutex_lock(&(This->mutexPool));
		while(This->taskQue == NULL){
			if (OneWorker->stop) break;
			pthread_cond_wait(&(This->isEmpty),&(This->mutexPool));
		}
		if (OneWorker->stop){
			pthread_mutex_unlock(&(This->mutexPool));
			return (void*)-1;
		}
		Task * task = This->taskQue;
		Remv(This->taskQue,task);
		pthread_mutex_unlock(&(This->mutexPool));
		//printf("%lld接受任务\n",OneWorker->pthreadId);
		task->func(task->arg);
	}
	free(OneWorker);
	pthread_exit(NULL);
}

int ThreadPool::DestroyPool(){
	workers* itor;
	for (itor = this->worker;itor!=NULL;itor = itor->next){
		itor->stop = 1;
	}
	pthread_mutex_lock(&(this->mutexPool));
	pthread_cond_broadcast(&(this->isEmpty));
	pthread_mutex_unlock(&(this->mutexPool));
	return 0;
}

int ThreadPool::TaskAdd(Task *task){

	pthread_mutex_lock(&(this->mutexPool));
	Addto(this->taskQue,task);
	pthread_cond_signal(&(this->isEmpty));
	pthread_mutex_unlock(&(this->mutexPool));

}

/*
void test(void *arg);
int main()
{
	ThreadPool*pool = new ThreadPool(3,20);
	Task task = {test,NULL,NULL,NULL};
	pool->TaskAdd(&task);
	sleep(1);
	pool->TaskAdd(&task);
	sleep(1);
	pool->TaskAdd(&task);
	sleep(1);
	pool->TaskAdd(&task);
	pool->DestroyPool();
}

void test(void *arg){
	for (int i = 0; i < 5; i++)
		printf("%dtest\n",getpid());
	
}*/