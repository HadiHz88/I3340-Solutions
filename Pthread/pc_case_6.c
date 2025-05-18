#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define BUFFER_LIMIT 100 // Total size of the buffer
#define CHUNK_SIZE 4     // Number of elements each producer or consumer handles
#define NUM_PRODUCERS 25
#define NUM_CONSUMERS 25

int Buffer_Index_Value = 0;
int Producers_Count = 0; // Tracks number of production chunks (round robin)
int Consumers_Count = 0; // Tracks number of consumption chunks (round robin)

char *Buffer_Queue; // Shared buffer

// Synchronization primitives
pthread_mutex_t lock;
pthread_cond_t can_produce;            // Signals all producers to begin producing
pthread_cond_t can_consume;            // Signals all consumers to begin consuming
pthread_cond_t can_produce_internally; // Used for round-robin sequencing of producers
pthread_cond_t can_consume_internally; // Used for round-robin sequencing of consumers

int prod_turn = 0; // 0 = producers' turn, 1 = consumers' turn

/**
 * Consumer thread function
 * Each consumer waits for its round-robin turn and consumes CHUNK_SIZE elements.
 */
void *Consumer(void *param)
{
  int id = *(int *)param;
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait for the consumer's turn in round-robin order
    while ((Consumers_Count % NUM_CONSUMERS) != id)
    {
      pthread_cond_wait(&can_consume_internally, &lock);
    }

    // Wait until it's the global consumers' turn
    while (prod_turn == 0)
    {
      pthread_cond_wait(&can_consume, &lock);
    }

    pthread_t thread_id;

    // Consume CHUNK_SIZE elements from the buffer
    for (int i = 0; i < CHUNK_SIZE; ++i)
    {
      thread_id = pthread_self();
      printf("Thread %lu - Consumer %d consumed: %c, Total: %d \n",
             (unsigned long)thread_id, id, Buffer_Queue[--Buffer_Index_Value], Buffer_Index_Value);
    }

    // Update consumption counter
    Consumers_Count++;

    // Notify other consumers to check if it's their turn
    pthread_cond_broadcast(&can_consume_internally);

    // If buffer is empty, allow producers to resume work
    if (Buffer_Index_Value == 0)
    {
      pthread_cond_broadcast(&can_produce); // Global signal to producers
      prod_turn = 0;                        // Change turn
    }

    pthread_mutex_unlock(&lock);
  }
  return NULL;
}

/**
 * Producer thread function
 * Each producer waits for its round-robin turn and produces CHUNK_SIZE elements.
 */
void *Producer(void *param)
{
  int id = *(int *)param;
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait for the producer's turn in round-robin order
    while ((Producers_Count % NUM_PRODUCERS) != id)
    {
      pthread_cond_wait(&can_produce_internally, &lock);
    }

    // Wait until it's the global producers' turn
    while (prod_turn == 1)
    {
      pthread_cond_wait(&can_produce, &lock);
    }

    pthread_t thread_id;

    // Produce CHUNK_SIZE elements into the buffer
    for (int i = 0; i < CHUNK_SIZE && Buffer_Index_Value < BUFFER_LIMIT; ++i)
    {
      thread_id = pthread_self();
      Buffer_Queue[Buffer_Index_Value++] = '@';
      printf("Thread %lu - Producer %d produced: %c, Total: %d ------ \n",
             (unsigned long)thread_id, id, '@', Buffer_Index_Value);
    }

    // Update production counter
    Producers_Count++;

    // Notify other producers to check if it's their turn
    pthread_cond_broadcast(&can_produce_internally);

    // If buffer is full, allow consumers to resume work
    if (Buffer_Index_Value == BUFFER_LIMIT)
    {
      pthread_cond_broadcast(&can_consume); // Global signal to consumers
      prod_turn = 1;                        // Change turn
    }

    pthread_mutex_unlock(&lock);
  }
  return NULL;
}

/**
 * Entry point of the program
 * Initializes threads, mutexes, and condition variables, then joins threads.
 */
int main()
{
  pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];
  int ids[NUM_PRODUCERS]; // Array to assign unique IDs to threads

  // Allocate buffer space
  Buffer_Queue = (char *)malloc(sizeof(char) * BUFFER_LIMIT);

  // Initialize mutex and condition variables
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&can_produce, NULL);
  pthread_cond_init(&can_consume, NULL);
  pthread_cond_init(&can_produce_internally, NULL);
  pthread_cond_init(&can_consume_internally, NULL);

  // Create producer threads
  for (int i = 0; i < NUM_PRODUCERS; i++)
  {
    ids[i] = i;
    pthread_create(&producers[i], NULL, Producer, &ids[i]);
  }

  // Create consumer threads
  for (int i = 0; i < NUM_CONSUMERS; i++)
  {
    ids[i] = i;
    pthread_create(&consumers[i], NULL, Consumer, &ids[i]);
  }

  // Join producer threads
  for (int i = 0; i < NUM_PRODUCERS; i++)
  {
    pthread_join(producers[i], NULL);
  }

  // Join consumer threads
  for (int i = 0; i < NUM_CONSUMERS; i++)
  {
    pthread_join(consumers[i], NULL);
  }

  // Cleanup
  free(Buffer_Queue);
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&can_produce);
  pthread_cond_destroy(&can_consume);
  pthread_cond_destroy(&can_produce_internally);
  pthread_cond_destroy(&can_consume_internally);

  return 0;
}
