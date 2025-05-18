package Ex2_CounterThreads;

import java.util.Scanner;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * A multithreaded Java program that increments a shared counter using multiple
 * threads.
 * Each thread calls the increment method a specified number of times.
 * The final value of the counter is printed after all threads finish execution.
 */
public class CounterThreads {

    /**
     * Shared counter object accessed and incremented by all threads.
     */
    public static Counter counter = new Counter();

    /**
     * Main method that initializes the number of threads and increments per thread,
     * starts all threads, waits for their completion, and prints the final counter
     * value.
     *
     * @param args command-line arguments (not used)
     * @throws InterruptedException if any thread is interrupted while waiting
     */
    public static void main(String[] args) throws InterruptedException {

        int nbTimes, nbThreads;
        Scanner input = new Scanner(System.in);

        System.out.println("Enter the number of times each task should run inc(): ");
        nbTimes = input.nextInt();

        System.out.println("Enter the number of threads that should be created: ");
        nbThreads = input.nextInt();

        Thread[] tasks = new Thread[nbThreads];

        // Create and start each thread
        for (int i = 0; i < nbThreads; i++) {
            tasks[i] = new CounterTask(nbTimes);
            tasks[i].start();
        }

        // Wait for all threads to finish
        for (int i = 0; i < nbThreads; i++) {
            tasks[i].join();
        }

        System.out.println("The final value of the counter = " + counter.getCount());

        input.close();
    }

    /**
     * A thread-safe counter class that uses a ReentrantLock to safely increment
     * a shared count variable from multiple threads.
     */
    static class Counter {
        private static final Lock lock = new ReentrantLock();
        private long count = 0;

        /**
         * Atomically increments the counter by 1 in a thread-safe manner.
         */
        void inc() {
            lock.lock();
            try {
                count = count + 1;
            } finally {
                lock.unlock();
            }
        }

        /**
         * Returns the current value of the counter.
         *
         * @return the count value
         */
        long getCount() {
            return count;
        }
    }

    /**
     * A thread class that performs a fixed number of increments on the shared
     * counter.
     */
    static class CounterTask extends Thread {
        private final int nbTimes;

        /**
         * Constructs a CounterTask that will increment the counter a given number of
         * times.
         *
         * @param n the number of times to increment the counter
         */
        public CounterTask(int n) {
            this.nbTimes = n;
        }

        /**
         * Executes the task: repeatedly calls the counter's increment method.
         */
        @Override
        public void run() {
            for (int i = 0; i < nbTimes; i++) {
                counter.inc();
            }
        }
    }
}
