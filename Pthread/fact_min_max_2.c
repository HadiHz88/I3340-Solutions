#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 10 // Size of the integer array

// Shared global variables
int X;      // Input integer for factorial calculation
int arr[N]; // Array of integers to find min and max

// Function declarations for thread routines
void *factorial(void *arg);
void *min(void *arg);
void *max(void *arg);
void *minMax(void *arg);

int main()
{
  pthread_t tid_factorial, tid_minMax; // Thread identifiers for factorial and minMax threads
  void *fact_result;                   // Pointer to receive factorial result from thread
  void *minMax_result;                 // Pointer to receive minMax results (array) from thread

  // Read integer X from user
  printf("Enter the integer X: ");
  scanf("%d", &X);

  // Read N integers into array arr
  printf("Enter %d integers for the array:\n", N);
  for (int i = 0; i < N; i++)
  {
    scanf("%d", &arr[i]);
  }

  // Create thread to compute factorial of X, no argument needed
  pthread_create(&tid_factorial, NULL, factorial, NULL);

  // Create thread to compute min and max of the array concurrently
  pthread_create(&tid_minMax, NULL, minMax, NULL);

  // Wait for factorial thread to finish and get pointer to result
  pthread_join(tid_factorial, &fact_result);

  // Wait for minMax thread to finish and get pointer to results array
  pthread_join(tid_minMax, &minMax_result);

  // Print factorial result (cast to long long pointer)
  printf("Factorial of %d is %lld\n", X, *(long long *)fact_result);

  // Print min and max results (cast to int pointer array)
  printf("Min: %d, Max: %d\n", ((int *)minMax_result)[0], ((int *)minMax_result)[1]);

  // Free dynamically allocated memory returned by threads
  free(fact_result);
  free(minMax_result);

  return 0;
}

/**
 * Thread function to compute factorial of X.
 * Allocates memory dynamically to store the factorial result,
 * calculates factorial iteratively, then returns pointer to result.
 */
void *factorial(void *arg)
{
  long long *fact = malloc(sizeof(long long)); // Allocate memory for factorial result
  *fact = 1;

  // Calculate factorial from 1 to X
  for (int i = 1; i <= X; i++)
  {
    *fact *= i;
  }

  pthread_exit((void *)fact); // Exit thread returning pointer to factorial result
}

/**
 * Thread function to find minimum element in global array arr[].
 * Allocates memory to store minimum value and returns pointer to it.
 */
void *min(void *arg)
{
  int *min_val = malloc(sizeof(int)); // Allocate memory for min value
  *min_val = arr[0];                  // Initialize with first element

  // Iterate array to find smallest element
  for (int i = 1; i < N; i++)
  {
    if (arr[i] < *min_val)
    {
      *min_val = arr[i];
    }
  }

  pthread_exit((void *)min_val); // Exit thread returning pointer to min value
}

/**
 * Thread function to find maximum element in global array arr[].
 * Allocates memory to store maximum value and returns pointer to it.
 */
void *max(void *arg)
{
  int *max_val = malloc(sizeof(int)); // Allocate memory for max value
  *max_val = arr[0];                  // Initialize with first element

  // Iterate array to find largest element
  for (int i = 1; i < N; i++)
  {
    if (arr[i] > *max_val)
    {
      *max_val = arr[i];
    }
  }

  pthread_exit((void *)max_val); // Exit thread returning pointer to max value
}

/**
 * Thread function to compute minimum and maximum values concurrently.
 * It creates two threads, one for min and one for max.
 * After both finish, it aggregates results in a dynamically allocated
 * integer array of size 2: results[0] = min, results[1] = max.
 * It frees intermediate allocations from min and max threads and
 * returns pointer to aggregated results.
 */
void *minMax(void *arg)
{
  pthread_t t_min, t_max; // Thread IDs for min and max threads
  void *min_result, *max_result;

  // Create thread to compute minimum value
  pthread_create(&t_min, NULL, min, NULL);

  // Create thread to compute maximum value
  pthread_create(&t_max, NULL, max, NULL);

  // Wait for min thread to complete and collect its result
  pthread_join(t_min, &min_result);

  // Wait for max thread to complete and collect its result
  pthread_join(t_max, &max_result);

  // Allocate array to hold both results
  int *results = malloc(2 * sizeof(int));
  results[0] = *(int *)min_result; // Store min value
  results[1] = *(int *)max_result; // Store max value

  // Free memory allocated by min and max threads
  free(min_result);
  free(max_result);

  pthread_exit((void *)results); // Return pointer to results array
}
