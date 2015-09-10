#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

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
	int i;
	while(1)
	{
		printf("Producer thread running...\n");
		pthread_mutex_lock(&mutex);
		printf("Producer thread has the lock...\n");
		while(myQueue.size >= maxSize)
		{
			printf("Producer thread waiting...\n");
			pthread_cond_wait(&upc,&mutex);
			printf("Producer thread awakens and receives lock...\n");
		}

		int r = rand() % 10;
		myQueue.nums[myQueue.size] = r;
		printf("Added %d to queue at position %d\n",r,myQueue.size);
		myQueue.size += 1;	
	
		printQueue(&myQueue);

		printf("Producer thread releasing lock and signaling...\n");
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&downc);

		//sleep(rand()%2);
	}
}

void *consume()
{
	int i;
	while(1)
	{
		printf("Consumer thread running...\n");
		pthread_mutex_lock(&mutex);
		printf("Consumer thread has the lock...\n");
		while(myQueue.size <= 0)
		{
			printf("Consumer thread waiting...\n");
			pthread_cond_wait(&downc,&mutex);
			printf("Consumer thread awakens and receives lock...\n");
		}

		//consume item from front of queue
		printf("Consumed %d from front of queue!\n",myQueue.nums[0]);

		//need to shift items in queue to the front	
		shiftQueue(&myQueue);
		myQueue.size -= 1;
		printQueue(&myQueue);

		printf("Consumer thread releasing lock and signaling...\n");
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&upc);

		//sleep(rand()%2);
	}
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	pthread_t consumer, producer;
	myQueue.size = 0;
	printf("\n");

	pthread_create(&producer, NULL, produce, NULL);
	pthread_create(&consumer, NULL, consume, NULL);
	
	pthread_join(producer, NULL);
	pthread_join(consumer, NULL);

	printf("Program finished running!\n");
}

