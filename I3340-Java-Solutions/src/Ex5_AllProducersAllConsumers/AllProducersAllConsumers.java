package Ex5_AllProducersAllConsumers;

import java.util.Date;
import java.util.LinkedList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * This class demonstrates a producer-consumer system where producers
 * and consumers coordinate not only across groups, but also **within**
 * their respective groups.
 *
 * It uses a turn-based system where producers and consumers take turns
 * to act collectively, enhancing group-level cooperation logic.
 *
 * Queue size is limited to 10 elements.
 */
public class AllProducersAllConsumers {

    // Shared queue
    public static MyQueue q = new MyQueue();

    public static void main(String[] args) {
        ExecutorService executor = Executors.newCachedThreadPool();

        // Launch producers and consumers
        for (int i = 1; i < 100; i++) {
            executor.execute(new Producer());
            executor.execute(new Consumer());
        }

        executor.shutdown();

        // Wait until all tasks are finished
        while (!executor.isTerminated()) {
            Thread.yield();
        }

        System.out.println("The final size of the queue = " + q.size());
    }

    /**
     * Producer thread that adds the current timestamp to the queue.
     */
    static class Producer extends Thread {
        @Override
        public void run() {
            try {
                Date d = new Date();
                q.addEl(d);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }

    /**
     * Consumer thread that removes a timestamp from the queue.
     */
    static class Consumer extends Thread {
        @Override
        public void run() {
            try {
                q.removeEl();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }

    /**
     * A shared queue supporting synchronized cooperative producer/consumer logic.
     * Uses explicit locks and multiple condition variables.
     */
    static class MyQueue extends LinkedList<Date> {

        private final Lock lock = new ReentrantLock();
        private final Condition notFull = lock.newCondition();
        private final Condition notEmpty = lock.newCondition();
        private final Condition producersTurn = lock.newCondition();
        private final Condition consumersTurn = lock.newCondition();

        // Alternates between producers and consumers
        private boolean isProducerTurn = true;

        private static final int CAPACITY = 10;

        /**
         * Adds an element to the queue if space is available and it's producers' turn.
         * Otherwise, the producer thread waits.
         *
         * @param e Date to be added
         * @throws InterruptedException if interrupted while waiting
         */
        public void addEl(Date e) throws InterruptedException {
            lock.lock();
            try {
                // Wait until it's the producers' turn and there is space
                while (!isProducerTurn || size() >= CAPACITY) {
                    if (!isProducerTurn)
                        producersTurn.await();
                    else
                        notFull.await();
                }

                super.addLast(e);
                System.out.println(e.toString() + " is added by Producer " + Thread.currentThread().threadId());

                // Switch turn to consumers and notify them
                isProducerTurn = false;
                consumersTurn.signalAll();
            } finally {
                lock.unlock();
            }
        }

        /**
         * Removes an element from the queue if it’s not empty and it’s consumers' turn.
         * Otherwise, the consumer thread waits.
         *
         * @throws InterruptedException if interrupted while waiting
         */
        public void removeEl() throws InterruptedException {
            lock.lock();
            try {
                // Wait until it's the consumers' turn and the queue is not empty
                while (isProducerTurn || size() == 0) {
                    if (isProducerTurn)
                        consumersTurn.await();
                    else
                        notEmpty.await();
                }

                Date d = super.removeFirst();
                System.out.println(d.toString() + " is removed by Consumer " + Thread.currentThread().threadId());

                // Switch turn to producers and notify them
                isProducerTurn = true;
                producersTurn.signalAll();
            } finally {
                lock.unlock();
            }
        }
    }
}
