package Ex4_StackThreads;

import static java.lang.Thread.sleep;

import java.util.LinkedList;
import java.util.NoSuchElementException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Demonstrates a thread-safe Stack implementation using LinkedList
 * with concurrent push and pop operations performed by multiple threads.
 * <p>
 * Synchronization is achieved using ReentrantLock and Condition to handle
 * blocking behavior for pop when the stack is empty.
 * <p>
 */
public class StackThreads {

    /**
     * Shared stack instance used by all threads.
     */
    private static Stack s = new Stack();

    /**
     * Main entry point.
     * Starts multiple push and pop threads via an ExecutorService.
     * Waits for all tasks to complete before printing final stack size.
     *
     * @param args command-line arguments (not used)
     * @throws InterruptedException if the main thread is interrupted while waiting
     */
    public static void main(String[] args) throws InterruptedException {

        // ExecutorService to manage thread pool for push/pop tasks
        ExecutorService executor = Executors.newCachedThreadPool();

        int c = 1;
        // Submit 1000 pop and push tasks alternately
        while (c < 1000) {
            executor.submit(new PopTask());
            // executor.submit(new IsEmptyTask()); // Uncomment if you want to test isEmpty
            // in parallel
            executor.submit(new PushTask());
            c++;
        }

        // Shutdown executor; no new tasks accepted after this call
        executor.shutdown();

        // Wait for all tasks to finish executing
        while (!executor.isTerminated()) {
            System.out.println("Main - Executor not terminated. The current size is " + s.size());
            Thread.yield(); // Yield to other threads while waiting
        }

        System.out.println("---------END OF MAIN --------------");
        System.out.println("The FINAL size is  " + s.size());
    }

    /**
     * Default constructor initializes a new Stack instance.
     */
    public StackThreads() {
        this.s = new Stack();
    }

    /**
     * Thread task for popping an element from the stack.
     */
    private static class PopTask extends Thread {
        @Override
        public void run() {
            s.pop();
        }
    }

    /**
     * Thread task for pushing an element ("A") onto the stack.
     */
    private static class PushTask extends Thread {
        @Override
        public void run() {
            s.push("A");
        }
    }

    /**
     * Thread task that checks whether the stack is empty and prints the result.
     */
    private static class IsEmptyTask extends Thread {
        @Override
        public void run() {
            if (s.isEmpty())
                System.out.println("The stack is empty");
            else
                System.out.println("The stack is not empty");
        }
    }

    /**
     * Thread-safe Stack class extending LinkedList.
     * <p>
     * Synchronizes push and pop methods using ReentrantLock and Condition.
     * The pop method blocks when the stack is empty until an element is pushed.
     */
    private static class Stack extends LinkedList {

        private final Lock lock = new ReentrantLock();
        private final Condition notEmpty = lock.newCondition();

        /**
         * Pushes an element onto the top of the stack.
         * Signals waiting pop threads that an element is available.
         *
         * @param e the element to push onto the stack
         */
        @Override
        public void push(Object e) {
            try {
                lock.lock();
                super.push(e);
                System.out.println("push");
                System.out.println("push - The size is  " + s.size());

                // Signal any waiting pop threads that stack is not empty
                notEmpty.signal();
            } finally {
                // Always release lock in finally block to prevent deadlocks
                lock.unlock();
            }
        }

        /**
         * Pops the top element from the stack.
         * If the stack is empty, waits until an element is pushed.
         *
         * @return the popped element, or null if interrupted during wait
         */
        @Override
        public Object pop() {
            Object o = null;
            long threadId = Thread.currentThread().threadId();
            try {
                lock.lock();

                // Use while loop to avoid spurious wakeups and race conditions
                while (s.isEmpty()) {
                    System.out.println("POP - Thread ID: " + threadId);
                    System.out.println("Waiting another thread to push an element");
                    try {
                        // Wait releases lock and suspends the thread until signaled
                        notEmpty.await();
                    } catch (InterruptedException ex) {
                        System.out.println("Interruption Exception in pop: " + ex);
                        Logger.getLogger(StackThreads.class.getName()).log(Level.SEVERE, null, ex);
                        Thread.currentThread().interrupt();
                        return null;
                    }
                }

                try {
                    o = super.pop();
                } catch (NoSuchElementException ex) {
                    System.out.println("Exception in pop: " + ex);
                }
                System.out.println("pop DONE");
                System.out.println("pop - The size is  " + s.size());

            } finally {
                lock.unlock();
            }
            return o;
        }

        /**
         * Checks if the stack is empty in a thread-safe manner.
         * Adds a 2-second sleep to simulate processing delay.
         *
         * @return true if the stack is empty, false otherwise
         */
        @Override
        public boolean isEmpty() {
            boolean b;
            lock.lock();
            try {
                b = super.isEmpty();
            } finally {
                lock.unlock();
            }
            try {
                sleep(5); // Simulate delay (careful: blocks thread!)
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
            return b;
        }
    }
}
