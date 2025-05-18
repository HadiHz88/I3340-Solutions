#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 10 // Size of the array for min/max calculations

// Shared global variables
int X;      // Input integer for factorial calculation
int arr[N]; // Array of integers for min and max computations

// Function declarations for thread routines
void *factorial(void *arg);
void *min(void *arg);
void *max(void *arg);
void *minMax(void *arg);

int main()
{
  pthread_t tid_factorial, tid_minMax; // Thread IDs for factorial and minMax threads
  long long factorial_result;          // Variable to store factorial result
  int results[2];                      // Array to store min and max results (results[0] = min, results[1] = max)

  // Prompt and read the integer X for factorial
  printf("Enter the integer X: ");
  scanf("%d", &X);

  // Prompt and read the array elements
  printf("Enter %d integers for the array:\n", N);
  for (int i = 0; i < N; i++)
  {
    scanf("%d", &arr[i]);
  }

  // Create a thread to compute factorial of X
  pthread_create(&tid_factorial, NULL, factorial, &factorial_result);

  // Create a thread to compute min and max values of the array concurrently
  pthread_create(&tid_minMax, NULL, minMax, results);

  // Wait for factorial thread to finish execution
  pthread_join(tid_factorial, NULL);

  // Wait for minMax thread to finish execution
  pthread_join(tid_minMax, NULL);

  // Display the results after threads complete their calculations
  printf("Factorial of %d is %lld\n", X, factorial_result);
  printf("Min: %d, Max: %d\n", results[0], results[1]);

  return 0;
}

/**
 * Thread function to compute factorial of the global variable X.
 * The result is stored in the long long pointer passed via arg.
 */
void *factorial(void *arg)
{
  long long *fact = (long long *)arg; // Cast argument to long long pointer
  *fact = 1;                          // Initialize factorial result to 1

  // Calculate factorial iteratively from 1 to X
  for (int i = 1; i <= X; i++)
  {
    *fact *= i;
  }

  pthread_exit(NULL); // Terminate thread; result is stored in *fact
}

/**
 * Thread function to find the minimum value in the global array arr[].
 * The minimum value is stored in results[0].
 */
void *min(void *arg)
{
  int *results = (int *)arg; // Cast argument to int pointer array
  int min_val = arr[0];      // Initialize min_val to first element of arr

  // Iterate through the array to find the smallest element
  for (int i = 1; i < N; i++)
  {
    if (arr[i] < min_val)
    {
      min_val = arr[i];
    }
  }

  results[0] = min_val; // Store minimum value in results[0]
  pthread_exit(NULL);   // Thread exits, result stored directly
}

/**
 * Thread function to find the maximum value in the global array arr[].
 * The maximum value is stored in results[1].
 */
void *max(void *arg)
{
  int *results = (int *)arg; // Cast argument to int pointer array
  int max_val = arr[0];      // Initialize max_val to first element of arr

  // Iterate through the array to find the largest element
  for (int i = 1; i < N; i++)
  {
    if (arr[i] > max_val)
    {
      max_val = arr[i];
    }
  }

  results[1] = max_val; // Store maximum value in results[1]
  pthread_exit(NULL);   // Thread exits, result stored directly
}

/**
 * Thread function to concurrently compute min and max values by creating two threads.
 * It uses the min() and max() functions in separate threads and waits for their completion.
 * Results are stored in the passed results array.
 */
void *minMax(void *arg)
{
  int *results = (int *)arg; // Cast argument to int pointer array
  pthread_t t_min, t_max;    // Thread IDs for min and max threads

  // Create thread to find minimum value
  pthread_create(&t_min, NULL, min, results);

  // Create thread to find maximum value
  pthread_create(&t_max, NULL, max, results);

  // Wait for both min and max threads to complete
  pthread_join(t_min, NULL);
  pthread_join(t_max, NULL);

  pthread_exit(NULL); // Exit thread after results are ready
}
