#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define BUFFER_LIMIT 250 // Maximum capacity of the buffer
#define CHUNK_SIZE 25    // Number of items each thread produces or consumes per iteration
#define NUM_PRODUCERS 5  // Total number of producer threads
#define NUM_CONSUMERS 5  // Total number of consumer threads

// Current number of items in the buffer
int Buffer_Index_Value = 0;

// Counters to track completed producer and consumer threads in each round
int Producers_Completed = 0;
int Consumers_Completed = 0;

// Dynamically allocated buffer to store produced characters
char *Buffer_Queue;

// Mutex and condition variables for synchronizing access to the buffer
pthread_mutex_t lock;
pthread_cond_t can_produce;
pthread_cond_t can_consume;

/**
 * Consumer thread function.
 * Waits for all producers to complete their batch, then consumes CHUNK_SIZE items.
 * Signals all producers once all consumers have completed.
 */
void *Consumer(void *param)
{
  pthread_t thread_id = pthread_self(); // Get the thread's ID

  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until all producers have completed their current batch
    while (Producers_Completed < NUM_PRODUCERS)
    {
      pthread_cond_wait(&can_consume, &lock);
    }

    // Consume CHUNK_SIZE elements
    for (int i = 0; i < CHUNK_SIZE; ++i)
    {
      printf("Consumer %lu consumed: %c, Total: %d\n",
             (unsigned long)thread_id,
             Buffer_Queue[--Buffer_Index_Value], // Consume from the end of buffer
             Buffer_Index_Value);
    }

    Consumers_Completed++;

    // If all consumers have finished, reset counters and wake up all producers
    if (Consumers_Completed == NUM_CONSUMERS)
    {
      Producers_Completed = 0;
      Consumers_Completed = 0;
      pthread_cond_broadcast(&can_produce);
    }

    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

/**
 * Producer thread function.
 * Waits until the buffer has space, then produces CHUNK_SIZE items.
 * Signals all consumers once all producers have completed.
 */
void *Producer(void *param)
{
  pthread_t thread_id = pthread_self(); // Get the thread's ID

  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until there's space in the buffer
    while (Buffer_Index_Value >= BUFFER_LIMIT)
    {
      pthread_cond_wait(&can_produce, &lock);
    }

    // Produce CHUNK_SIZE items into the buffer
    for (int i = 0; i < CHUNK_SIZE; ++i)
    {
      Buffer_Queue[Buffer_Index_Value++] = '@';
      printf("Producer %lu produced: %c, Total: %d\n",
             (unsigned long)thread_id,
             '@',
             Buffer_Index_Value);
    }

    Producers_Completed++;

    // If all producers have finished, wake up all consumers
    if (Producers_Completed == NUM_PRODUCERS)
    {
      pthread_cond_broadcast(&can_consume);
    }

    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

// Initializes shared resources and spawns producer and consumer threads. Waits for all threads to join (infinite in this design).
int main()
{
  pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];

  // Allocate the buffer
  Buffer_Queue = (char *)malloc(sizeof(char) * BUFFER_LIMIT);

  // Initialize synchronization primitives
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&can_produce, NULL);
  pthread_cond_init(&can_consume, NULL);

  // Create all producer threads
  for (int i = 0; i < NUM_PRODUCERS; ++i)
  {
    pthread_create(&producers[i], NULL, Producer, NULL);
  }

  // Create all consumer threads
  for (int i = 0; i < NUM_CONSUMERS; ++i)
  {
    pthread_create(&consumers[i], NULL, Consumer, NULL);
  }

  // Wait for all threads (these threads run infinitely, so this blocks forever)
  for (int i = 0; i < NUM_PRODUCERS; ++i)
  {
    pthread_join(producers[i], NULL);
  }
  for (int i = 0; i < NUM_CONSUMERS; ++i)
  {
    pthread_join(consumers[i], NULL);
  }

  // Cleanup (unreachable in current code due to infinite loops)
  free(Buffer_Queue);
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&can_produce);
  pthread_cond_destroy(&can_consume);

  return 0;
}
