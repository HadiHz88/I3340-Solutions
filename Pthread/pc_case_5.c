#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define BUFFER_LIMIT 250 // Total size of the buffer
#define CHUNK_SIZE 5     // Number of elements each producer or consumer handles
#define NUM_PRODUCERS 50
#define NUM_CONSUMERS 50

int Buffer_Index_Value = 0; // Index tracker for buffer position
char *Buffer_Queue;         // Shared buffer between producers and consumers

// Synchronization primitives
pthread_mutex_t lock;
pthread_cond_t can_produce;          // Signaled when producers are allowed to work
pthread_cond_t can_consume;          // Signaled when consumers are allowed to work
pthread_cond_t internal_can_produce; // Used to allow internal signaling among producers
pthread_cond_t internal_can_consume; // Used to allow internal signaling among consumers

int prod_turn = 0; // Flag to determine turn: 0 = producers, 1 = consumers

/**
 * Consumer thread function
 * Consumes CHUNK_SIZE elements from the shared buffer
 * Waits for the buffer to be full before consuming
 */
void *Consumer()
{
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until it's the consumers' turn
    while (prod_turn == 0)
    {
      pthread_cond_wait(&can_consume, &lock);
    }

    pthread_t thread_id;

    // Consume CHUNK_SIZE elements
    for (int i = 0; i < CHUNK_SIZE; ++i)
    {
      thread_id = pthread_self();
      printf("Thread %lu - Consumer consumed: %c, Total: %d \n",
             (unsigned long)thread_id, Buffer_Queue[--Buffer_Index_Value], Buffer_Index_Value);
    }

    // If buffer is fully consumed, allow producers to resume
    if (Buffer_Index_Value == 0)
    {
      pthread_cond_broadcast(&can_produce); // Wake up all producers
      prod_turn = 0;                        // Switch turn to producers
    }

    pthread_cond_broadcast(&internal_can_consume);   // Signal to other consumers
    pthread_cond_wait(&internal_can_consume, &lock); // Wait for another consumer to proceed

    pthread_mutex_unlock(&lock);
  }
  return NULL;
}

/**
 * Producer thread function
 * Produces CHUNK_SIZE elements into the shared buffer
 * Waits for the buffer to be empty before producing
 */
void *Producer()
{
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until it's the producers' turn
    while (prod_turn == 1)
    {
      pthread_cond_wait(&can_produce, &lock);
    }

    pthread_t thread_id;

    // Produce CHUNK_SIZE elements
    for (int i = 0; i < CHUNK_SIZE && Buffer_Index_Value < BUFFER_LIMIT; ++i)
    {
      thread_id = pthread_self();
      Buffer_Queue[Buffer_Index_Value++] = '@';
      printf("Thread %lu - Producer produced: %c, Total: %d\n",
             (unsigned long)thread_id, '@', Buffer_Index_Value);
    }

    // If buffer is fully filled, allow consumers to resume
    if (Buffer_Index_Value == BUFFER_LIMIT)
    {
      pthread_cond_broadcast(&can_consume); // Wake up all consumers
      prod_turn = 1;                        // Switch turn to consumers
    }

    pthread_cond_broadcast(&internal_can_produce);   // Signal to other producers
    pthread_cond_wait(&internal_can_produce, &lock); // Wait for another producer to proceed

    pthread_mutex_unlock(&lock);
  }
  return NULL;
}

int main()
{
  pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];

  // Allocate memory for the buffer
  Buffer_Queue = (char *)malloc(sizeof(char) * BUFFER_LIMIT);

  // Initialize mutex and condition variables
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&can_produce, NULL);
  pthread_cond_init(&can_consume, NULL);
  pthread_cond_init(&internal_can_consume, NULL);
  pthread_cond_init(&internal_can_produce, NULL);

  // Create producer threads
  for (int i = 0; i < NUM_PRODUCERS; i++)
  {
    pthread_create(&producers[i], NULL, Producer, NULL);
  }

  // Create consumer threads
  for (int i = 0; i < NUM_CONSUMERS; i++)
  {
    pthread_create(&consumers[i], NULL, Consumer, NULL);
  }

  // Wait for all producer threads to finish
  for (int i = 0; i < NUM_PRODUCERS; i++)
  {
    pthread_join(producers[i], NULL);
  }

  // Wait for all consumer threads to finish
  for (int i = 0; i < NUM_CONSUMERS; i++)
  {
    pthread_join(consumers[i], NULL);
  }

  // Clean up resources
  free(Buffer_Queue);
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&can_produce);
  pthread_cond_destroy(&can_consume);
  pthread_cond_destroy(&internal_can_produce);
  pthread_cond_destroy(&internal_can_consume);

  return 0;
}
