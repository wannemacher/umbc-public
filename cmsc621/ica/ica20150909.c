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

/**
 * Pushes the integer onto the end of the queue.
 *
 * @param q the queue onto which to push the integer
 * @param x the integer
 */
void enqueue(struct queue * q, int x)
{
    struct node * n = (struct node *) malloc(sizeof(struct node));
    n->data = x;
    n->next = NULL;

    // Special case if the queue is empty
    if(q->head == NULL && q->tail == NULL)
    {
        q->head = n;
        q->tail = n;
    }
    else
    {
        q->tail->next = n;
        q->tail = n;
    }

    q->size++;
}

/**
 * Pops the integer off the front of the queue.
 *
 * @param q the queue
 * @return the integer at the front of the queue
 */
struct node * dequeue(struct queue * q)
{
    struct node * n = q->head;

    // Special case if the queue has one item
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

    return n;
}

/**
 * Dequeues all the integers in the queue and frees the queue memory.
 *
 * @param q the queue to destroy
 */
void destroy_queue(struct queue * q)
{
    while (q->size > 0)
    {
        struct node * n = dequeue(q);
        free(n);
    }

    free(q);
}

/**
 * Prints the queue.
 *
 * @param q the queue
 */
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

/**
 * The main function of the producer thread. Generates a random integer and
 * enqueues it onto a shared queue as long as the queue isn't full.
 *
 * @return NULL
 */
void  * produce()
{
    srand(time(NULL));

    int i, x;
    for (i = 0; i < MAX_ITERATIONS; i++)
    {
        x = rand() % 100;

        // Wait for the queue to not be full then grab the lock
        pthread_mutex_lock(&mutex);
        while (integers->size >= MAX_QUEUE_SIZE)
        {
            pthread_cond_wait(&cv_queue_not_full, &mutex);
        }

        enqueue(integers, x);

        printf("PRODUCE: ");
        print_queue(integers);
        printf("\n");

        // Signal that the queue isn't empty and release the lock
        pthread_cond_signal(&cv_queue_not_empty);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/**
 * The main function of the consumer thread. Dequeues an integer from a shared
 * queue as long as the queue isn't empty.
 *
 * @return NULL
 */
void * consume()
{
    int i;
    for (i = 0; i < MAX_ITERATIONS; i++)
    {
        // Wait for the queue to not be empty then grab the lock
        pthread_mutex_lock(&mutex);
        while (integers->size <= 0)
        {
            pthread_cond_wait(&cv_queue_not_empty, &mutex);
        }

        struct node * n = dequeue(integers);
        free(n);

        printf("CONSUME: ");
        print_queue(integers);
        printf("\n");

        // Signal that the queue isn't full and release the lock
        pthread_cond_signal(&cv_queue_not_full);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/**
 * The main function. Initializes a shared queue and creates a producer thread
 * and a consumer thread.
 *
 * @return 0 on success
 */
int main(int argc, char ** argv)
{
    int status = 0;
    pthread_t producer, consumer;

    pthread_mutex_init(&mutex, NULL);

    integers = (struct queue *) malloc(sizeof(struct queue));
    if (integers == NULL)
    {
        perror("Error : Unable to allocate memory for the integer queue ");
        status++;
        goto ret;
    }

    integers->size = 0;
    integers->head = NULL;
    integers->tail = NULL;

    // Create the producer and consumer threads
    status += pthread_create(&producer, NULL, produce, NULL);
    status += pthread_create(&consumer, NULL, consume, NULL);
    if (status != 0)
    {
        perror("Error : Unable to create producer/consumer threads ");
        goto ret;
    }

    // Wait for the producer and consumer threads to terminate
    status += pthread_join(producer, NULL);
    status += pthread_join(consumer, NULL);
    if (status != 0)
    {
        perror("Error : Unable to join producer/consumer threads ");
        goto ret;
    }

    destroy_queue(integers);

ret:
    return status;
}
