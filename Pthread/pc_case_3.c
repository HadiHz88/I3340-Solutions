#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define BUFFER_LIMIT 10 // Maximum capacity of the buffer
#define CHUNK_SIZE 3    // Number of items to produce or consume per operation

// Index representing the current number of items in the buffer
int Buffer_Index_Value = 0;

// Dynamically allocated buffer to hold characters
char *Buffer_Queue;

// Mutex for synchronizing access to the buffer
pthread_mutex_t lock;

// Condition variable to signal when production is allowed
pthread_cond_t can_produce;

// Condition variable to signal when consumption is allowed
pthread_cond_t can_consume;

/**
 * Consumer thread function
 * Waits for at least one item in the buffer, then consumes CHUNK_SIZE elements.
 * After consuming, it signals the producer to continue if there is space.
 */
void *Consumer(void *param)
{
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until there are elements available in the buffer
    while (Buffer_Index_Value == 0)
    {
      pthread_cond_wait(&can_consume, &lock);
    }

    pthread_t thread_id;

    // Consume CHUNK_SIZE items from the buffer
    for (int i = 0; i < CHUNK_SIZE; ++i)
    {
      thread_id = pthread_self();
      printf("Thread %lu - Consumer consumed: %c, Total: %d\n",
             (unsigned long)thread_id,
             Buffer_Queue[--Buffer_Index_Value], // Consume from the end of the buffer
             Buffer_Index_Value);
    }

    // Signal the producer that space is available to produce more items
    pthread_cond_signal(&can_produce);
    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

/**
 * Producer thread function
 * Waits until enough space is available, then produces CHUNK_SIZE elements.
 * After producing, it signals the consumer that items are available.
 */
void *Producer(void *param)
{
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until there is room to produce at least CHUNK_SIZE items
    while (Buffer_Index_Value == BUFFER_LIMIT)
    {
      pthread_cond_wait(&can_produce, &lock);
    }

    pthread_t thread_id;

    // Produce CHUNK_SIZE items and append them to the buffer
    for (int i = 0; i < CHUNK_SIZE; ++i)
    {
      Buffer_Queue[Buffer_Index_Value++] = '@'; // Produce item
      thread_id = pthread_self();
      printf("Thread %lu - Producer produced: %c, Total: %d\n",
             (unsigned long)thread_id,
             '@',
             Buffer_Index_Value);
    }

    // Signal the consumer that new items are available
    pthread_cond_signal(&can_consume);
    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

// Initializes the buffer and synchronization primitives, spawns producer and consumer threads, and waits for them to complete (though they run infinitely).
int main()
{
  pthread_t producer_thread_id, consumer_thread_id;

  // Allocate memory for the buffer
  Buffer_Queue = (char *)malloc(sizeof(char) * BUFFER_LIMIT);

  // Initialize mutex and condition variables
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&can_produce, NULL);
  pthread_cond_init(&can_consume, NULL);

  // Create producer and consumer threads
  pthread_create(&producer_thread_id, NULL, Producer, NULL);
  pthread_create(&consumer_thread_id, NULL, Consumer, NULL);

  // Wait for both threads (not reachable due to infinite loops)
  pthread_join(producer_thread_id, NULL);
  pthread_join(consumer_thread_id, NULL);

  // Cleanup (not reached, but good practice)
  free(Buffer_Queue);
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&can_produce);
  pthread_cond_destroy(&can_consume);

  return 0;
}
