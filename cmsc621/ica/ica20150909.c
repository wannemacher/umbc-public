#include <pthread.h>
#include <stdio.h>

static const int MAX_QUEUE_SIZE = 10

pthread_mutex_t mutex;
pthread_cond_t cv_enqueue, cv_dequeue;

struct node
{
    int data;
    struct node * next;
};

struct node front = NULL;
struct node tail = NULL;
int size = 0;

void produce()
{
    struct node * temp = (struct node *) malloc(sizeof(struct node));
    temp->data = x;
    temp->next = NULL;

    pthread_mutex_lock(&mutex);
    while (size > MAX_QUEUE_SIZE)
    {
        
    }

    size++;

    if(front == NULL && rear == NULL)
    {
        front = rear = temp;
        return;
    }

    rear->next = temp;
    rear = temp;
}

void consume()
{
    struct node * temp = front;

    size--;

    if(front == rear)
    {
        front = rear = NULL;
    }
    else
    {
        front = front->next;
    }

    free(temp);
}

int main(int argc, char ** argv)
{
    pthread_t producer, consumer;

    pthread_create(&producer, NULL, produce, NULL);
    pthread_create(&consumer, NULL, consume, NULL);
	
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
}
