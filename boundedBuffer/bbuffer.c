/*
 * Mark Lotts
 * CMSC 621
 * 9/9/2015
 * In-Class Bounded Buffer Assignment
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct queue NumQueue;

//queue for storing bounded bufffer
struct queue
{
    int size;
    int nums[10];
};

//global variables
NumQueue myQueue;
static const int MAXSIZE = 10;
pthread_mutex_t mutex;
pthread_cond_t notfull, notempty;

//print out the queue
void printQueue(NumQueue* aQueue)
{
    int i;
    for(i=0;i<aQueue->size;i++)
    {
        printf("%d ",aQueue->nums[i]);
    }
    for(i=aQueue->size;i<MAXSIZE;i++)
    {
        printf("_ ");
    }
    printf("\n\n");
}

//move elements in the queue up one position
void shiftQueue(NumQueue* aQueue)
{
    int i;
    for(i=0;i<(aQueue->size)-1;i++)
    {
        aQueue->nums[i] = aQueue->nums[i+1];
    }
}

//run by producer thread to add items to the queue
void *produce()
{
    //have producer continually try and produce
    while(1)
    {
        //producer attempts to grab the lock
        printf("Producer thread running...\n");
        pthread_mutex_lock(&mutex);
        printf("Producer thread has the lock...\n");

        //if the queue is full, producer waits to be signaled
        while(myQueue.size >= MAXSIZE)
        {
            printf("Producer thread waiting...\n");
            pthread_cond_wait(&notfull,&mutex);
            printf("Producer thread awakens and receives lock...\n");
        }

        //generate a random number to put in queue (for simplicity's sake, limited to 0-9)
        int r = rand() % 10;

        //add item to queue
        myQueue.nums[myQueue.size] = r;
        printf("Added %d to queue at position %d\n",r,myQueue.size);
        myQueue.size += 1;

        printQueue(&myQueue);

        //release lock and signal that the buffer isn't empty
        printf("Producer thread releasing lock and signaling...\n");
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&notempty);
    }
}

//run by consumer thread to remove items from front of queue
void *consume()
{
    //have consumer continuously try and consume
    while(1)
    {
        //consumer attempts to grab the lock
        printf("Consumer thread running...\n");
        pthread_mutex_lock(&mutex);
        printf("Consumer thread has the lock...\n");

        //if the queue is empty, consumer waits to be signaled
        while(myQueue.size <= 0)
        {
            printf("Consumer thread waiting...\n");
            pthread_cond_wait(&notempty,&mutex);
            printf("Consumer thread awakens and receives lock...\n");
        }

        //consume item from front of queue
        printf("Consumed %d from front of queue!\n",myQueue.nums[0]);

        //need to shift items in queue to the front
        shiftQueue(&myQueue);
        myQueue.size -= 1;
        printQueue(&myQueue);

        //release lock and signal that the buffer isn't full
        printf("Consumer thread releasing lock and signaling...\n");
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&notfull);
    }
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    pthread_t consumer, producer;
    myQueue.size = 0;
    printf("\n");

    //create producer and consumer threads
    pthread_create(&producer, NULL, produce, NULL);
    pthread_create(&consumer, NULL, consume, NULL);

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    printf("Program finished running!\n");

    return 0;
}
