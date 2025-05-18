#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Maximum capacity of the buffer
#define BUFFER_LIMIT 10

// Shared buffer index — represents the current number of items in the buffer
int buffer_index = 0;

// Dynamically allocated character buffer
char *buffer;

// Mutex for protecting shared resources
pthread_mutex_t lock;

// Condition variable to signal when the buffer becomes ready for consumption
pthread_cond_t buffer_not_empty;

// Condition variable to signal when the buffer becomes ready for production
pthread_cond_t buffer_not_full;

// Flag to indicate the current state of the buffer:
// 1 = ready to be produced, 0 = ready to be consumed
int flag = 1;

/**
 * Consumer thread function
 * Waits until the buffer is fully produced, then consumes one item at a time.
 * Once the buffer is fully consumed, it signals the producer to start again.
 */
void *Consumer(void *param)
{
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until the buffer is ready to be consumed
    while (flag == 1)
    {
      pthread_cond_wait(&buffer_not_empty, &lock);
    }

    // Consume one item
    buffer_index--;
    printf("Consumer consumed: %d elements\n", buffer_index);

    // If all items have been consumed, notify producer to refill the buffer
    if (buffer_index == 0)
    {
      flag = 1;
      pthread_cond_signal(&buffer_not_full);
    }

    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

/**
 * Producer thread function
 * Waits until the buffer is empty, then produces items into it.
 * Once the buffer reaches full capacity, it signals the consumer to start consuming.
 */
void *Producer(void *param)
{
  while (1)
  {
    pthread_mutex_lock(&lock);

    // Wait until the buffer is ready to be produced
    while (flag == 0)
    {
      pthread_cond_wait(&buffer_not_full, &lock);
    }

    // Produce one item
    buffer[buffer_index++] = '@';
    printf("Producer produced: %d elements\n", buffer_index);

    // Once the buffer is full, notify consumer
    if (buffer_index == BUFFER_LIMIT)
    {
      flag = 0;
      pthread_cond_signal(&buffer_not_empty);
    }

    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

// Initializes synchronization primitives, creates producer and consumer threads, and waits for them (though they run infinitely).
int main()
{
  pthread_t producer_tid, consumer_tid;

  // Allocate memory for the buffer
  buffer = (char *)malloc(sizeof(char) * BUFFER_LIMIT);
  if (buffer == NULL)
  {
    perror("Failed to allocate buffer memory");
    return EXIT_FAILURE;
  }

  // Initialize synchronization primitives
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&buffer_not_empty, NULL);
  pthread_cond_init(&buffer_not_full, NULL);

  // Create producer and consumer threads
  pthread_create(&producer_tid, NULL, Producer, NULL);
  pthread_create(&consumer_tid, NULL, Consumer, NULL);

  // Wait for threads to finish (this will not happen due to infinite loops)
  pthread_join(producer_tid, NULL);
  pthread_join(consumer_tid, NULL);

  // Cleanup (unreachable, but good practice)
  free(buffer);
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&buffer_not_empty);
  pthread_cond_destroy(&buffer_not_full);

  return 0;
}
