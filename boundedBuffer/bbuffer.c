#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct queue NumQueue;

struct queue
{
	int size;
	int nums[10];
};


//global variables
NumQueue myQueue;

int maxSize = 10;
pthread_mutex_t mutex;
pthread_cond_t upc, downc;

void printQueue(NumQueue* aQueue)
{
	int i;
	for(i=0;i<aQueue->size;i++)
	{
		printf("%d ",aQueue->nums[i]);
	}
	for(i=aQueue->size;i<maxSize;i++)
	{
		printf("_ ");
	}
	printf("\n\n");
}

void shiftQueue(NumQueue* aQueue)
{
	int i;
	for(i=0;i<(aQueue->size)-1;i++)
	{
		aQueue->nums[i] = aQueue->nums[i+1];
	}
}

void *produce()
{
	printf("Producer thread running...\n");

	pthread_mutex_lock(&mutex);
	while(myQueue.size >= maxSize)
	{
		pthread_cond_wait(&upc,&mutex);
	}

	int r = rand() % 10;
	myQueue.nums[myQueue.size] = r;
	printf("Added item %d to queue at position %d\n",r,myQueue.size);
	myQueue.size += 1;	
	
	printQueue(&myQueue);

	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&downc);
}

void *consume()
{
	printf("Consumer thread\n");
	pthread_mutex_lock(&mutex);
	while(myQueue.size <= 0)
	{
		pthread_cond_wait(&downc,&mutex);
	}

	//consume item from front of queue
	printf("Consumed %d from front of queue!\n",myQueue.nums[0]);
	myQueue.size -= 1;

	//need to shift items in queue to the front	
	shiftQueue(&myQueue);
	printQueue(&myQueue);

	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&upc);
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	pthread_t consumer, producer;
	//pthread_attr_t attr;
	//size_t stacksize;
	//struct argument a1, a2;
	myQueue.size = 0;
	printf("\n");
	pthread_create(&producer, NULL, produce, NULL);//(void *)&a1);
	//pthread_create(&consumer, NULL, consume, NULL);//(void *)&a2);
	
	pthread_join(producer, NULL);
	//pthread_join(consumer, NULL);
	//printf("%d\n", count);
	printf("Program finished running!\n");
}

