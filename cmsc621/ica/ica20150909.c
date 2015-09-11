#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const int MAX_QUEUE_SIZE = 10;
static const int MAX_ITERATIONS = 100000;

pthread_mutex_t mutex;
pthread_cond_t cv_queue_not_full, cv_queue_not_empty;

struct node
{
    int data;
    struct node * next;
};

struct queue
{
    int size;
    struct node * head;
    struct node * tail;
};

struct queue * integers;

void enqueue(struct queue * q, int x)
{
    struct node * temp = (struct node *) malloc(sizeof(struct node));
    temp->data = x;
    temp->next = NULL;

    if(q->head == NULL && q->tail == NULL)
    {
        q->head = temp;
        q->tail = temp;
    }
    else
    {
        q->tail->next = temp;
        q->tail = temp;
    }

    q->size++;
}

void dequeue(struct queue * q)
{
    struct node * temp = q->head;

    if(q->head == q->tail)
    {
        q->head = NULL;
        q->tail = NULL;
    }
    else
    {
        q->head = q->head->next;
    }

    q->size--;
    free(temp);
}

void destroy_queue(struct queue * q)
{
    while (q->size > 0)
    {
        dequeue(q);
    }

    free(q);
}

void print_queue(struct queue * q)
{
    printf("(%02d) [ ", q->size);

    struct node * n = q->head;
    while (n != NULL)
    {
        printf("%d ", n->data);
        n = n->next;
    }

    printf("]");
}

void  * produce()
{
    srand(time(NULL));

    int i, x;
    for (i = 0; i < MAX_ITERATIONS; i++)
    {
        x = rand() % 100;

        pthread_mutex_lock(&mutex);
        while (integers->size >= MAX_QUEUE_SIZE)
        {
            pthread_cond_wait(&cv_queue_not_full, &mutex);
        }

        enqueue(integers, x);

        printf("PRODUCE: ");
        print_queue(integers);
        printf("\n");

        pthread_cond_signal(&cv_queue_not_empty);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void * consume()
{
    srand(time(NULL));

    int i;
    for (i = 0; i < MAX_ITERATIONS; i++)
    {
        pthread_mutex_lock(&mutex);
        while (integers->size <= 0)
        {
            pthread_cond_wait(&cv_queue_not_empty, &mutex);
        }

        dequeue(integers);

        printf("CONSUME: ");
        print_queue(integers);
        printf("\n");

        pthread_cond_signal(&cv_queue_not_full);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char ** argv)
{
    pthread_t producer, consumer;

    pthread_mutex_init(&mutex, NULL);

    integers = (struct queue *) malloc(sizeof(struct queue));
    integers->size = 0;
    integers->head = NULL;
    integers->tail = NULL;

    pthread_create(&producer, NULL, produce, NULL);
    pthread_create(&consumer, NULL, consume, NULL);
	
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    destroy_queue(integers);

    return 0;
}
