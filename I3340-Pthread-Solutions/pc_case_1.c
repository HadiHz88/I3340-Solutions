#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

// Maximum capacity of the shared buffer
#define BUFFER_LIMIT 100

// Shared buffer index — represents the current number of items in the buffer
int buffer_index = 0;

// Dynamically allocated character buffer
char *buffer;

// Mutex for mutual exclusion when accessing shared resources
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Condition variable to signal when the buffer becomes non-empty (used by consumers)
pthread_cond_t buffer_not_empty = PTHREAD_COND_INITIALIZER;

// Condition variable to signal when the buffer becomes non-full (used by producers)
pthread_cond_t buffer_not_full = PTHREAD_COND_INITIALIZER;

/**
 * Consumer thread function
 * Continuously consumes items from the shared buffer.
 * Waits when the buffer is empty until a producer signals that data is available.
 */
void *Consumer()
{
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until there is something to consume
    if (buffer_index == 0)
    {
      pthread_cond_wait(&buffer_not_empty, &lock);
    }

    // Consume one item by decrementing the buffer index
    buffer_index--;

    printf("Consumed. Items in buffer: %d\n", buffer_index);

    // Signal to producers that space is available in the buffer
    pthread_cond_signal(&buffer_not_full);

    pthread_mutex_unlock(&lock);

    // Optional sleep for realism / pacing
    // usleep(1000);
  }

  return NULL;
}

/**
 * Producer thread function
 * Continuously produces items into the shared buffer.
 * Waits when the buffer is full until a consumer signals that space is available.
 */
void *Producer()
{
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until there is space to produce
    if (buffer_index == BUFFER_LIMIT)
    {
      pthread_cond_wait(&buffer_not_full, &lock);
    }

    // Produce one item by incrementing the buffer index
    buffer_index++;
    buffer[buffer_index] = '@'; // Simulated produced item

    printf("Produced. Items in buffer: %d\n", buffer_index);

    // Signal to consumers that an item is available to consume
    pthread_cond_signal(&buffer_not_empty);

    pthread_mutex_unlock(&lock);

    // Optional sleep for realism / pacing
    // usleep(1000);
  }

  return NULL;
}

// Initializes the buffer, creates producer and consumer threads, and waits for their completion.
int main()
{
  pthread_t producer_tid, consumer_tid;

  // Allocate buffer memory
  buffer = (char *)malloc(sizeof(char) * BUFFER_LIMIT);
  if (buffer == NULL)
  {
    perror("Failed to allocate buffer memory");
    return EXIT_FAILURE;
  }

  // Create producer and consumer threads
  pthread_create(&producer_tid, NULL, Producer, NULL);
  pthread_create(&consumer_tid, NULL, Consumer, NULL);

  // Wait for threads to finish (they never do in this infinite-loop setup)
  pthread_join(producer_tid, NULL);
  pthread_join(consumer_tid, NULL);

  // Clean up (technically unreachable here)
  free(buffer);

  // Destroy mutex and condition variables
  // (not reached in this infinite loop, but good practice)
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&buffer_not_empty);
  pthread_cond_destroy(&buffer_not_full);

  return 0;
}
