package Ex6_ProducerConsumer;

import java.util.LinkedList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * ProducerConsumer demonstrates a classic producer-consumer problem implementation
 * using a shared queue protected by explicit locking with conditions for synchronization.
 * Producers add elements to the queue while consumers remove elements.
 * The queue has a fixed capacity of 10 elements.
 */
public class ProducerConsumer {

    // Lock for synchronizing access to the shared queue
    private static final Lock lock = new ReentrantLock();

    // Condition to signal when the queue is not empty (consumers can proceed)
    private static final Condition isEmpty = lock.newCondition();

    // Condition to signal when the queue is not full (producers can proceed)
    private static final Condition isFull = lock.newCondition();

    // Shared queue instance for producers and consumers
    public static MyQueue q = new MyQueue();

    /**
     * Main method initializes and runs producer and consumer tasks using a cached thread pool.
     * It submits 1000 pairs of producer and consumer tasks.
     * Waits for all tasks to finish before printing final queue size.
     *
     * @param args unused
     */
    public static void main(String[] args) {
        ExecutorService executor = Executors.newCachedThreadPool();

        int c = 1;
        while (c < 1000) {
            executor.execute(new Producer());
            executor.execute(new Consumer());
            c++;
        }

        executor.shutdown();

        // Wait until all tasks have completed execution
        while (!executor.isTerminated()) {
            Thread.yield();
            System.out.println("Current queue size: " + q.size());
        }

        System.out.println("---- End of MAIN ----");
        System.out.println("Final queue size: " + q.size());
    }

    /**
     * Producer thread that attempts to add an element "A" to the shared queue.
     * If the queue is full, it will wait until space becomes available.
     */
    static class Producer extends Thread {
        @Override
        public void run() {
            String s = "A";
            try {
                q.addElement(s);
            } catch (InterruptedException e) {
                // Restore interrupted status and log if necessary
                Thread.currentThread().interrupt();
            }
        }
    }

    /**
     * Consumer thread that attempts to remove an element from the shared queue.
     * If the queue is empty, it will wait until an element is available.
     */
    static class Consumer extends Thread {
        @Override
        public void run() {
            try {
                q.removeElement();
            } catch (InterruptedException e) {
                // Restore interrupted status and log if necessary
                Thread.currentThread().interrupt();
            }
        }
    }

    /**
     * Inner class representing a thread-safe queue with capacity limit.
     * It extends LinkedList and uses explicit locking and condition variables
     * to coordinate producers and consumers.
     */
    static class MyQueue extends LinkedList<String> {
        private static final int CAPACITY = 10;

        /**
         * Adds an element to the end of the queue.
         * If the queue is full, waits until space becomes available.
         *
         * @param s the element to add
         * @throws InterruptedException if the thread is interrupted while waiting
         */
        public void addElement(String s) throws InterruptedException {
            lock.lock();
            try {
                // Wait until queue has space to add new element
                while (size() == CAPACITY) {
                    isFull.await();
                }
                super.addLast(s);
                System.out.println(s + " is added");
                // Signal consumers waiting for non-empty queue
                isEmpty.signal();
            } finally {
                lock.unlock();
            }
        }

        /**
         * Removes and returns the first element from the queue.
         * If the queue is empty, waits until an element becomes available.
         *
         * @throws InterruptedException if the thread is interrupted while waiting
         */
        public void removeElement() throws InterruptedException {
            lock.lock();
            try {
                // Wait until queue is not empty
                while (size() == 0) {
                    isEmpty.await();
                }
                super.removeFirst();
                System.out.println("An element was removed");
                // Signal producers waiting for space in the queue
                isFull.signal();
            } finally {
                lock.unlock();
            }
        }
    }
}
