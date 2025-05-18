package Ex5_AllProducersAllConsumers;

import java.util.Date;
import java.util.LinkedList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * This class demonstrates a version of the Producer-Consumer problem
 * with additional cooperation between:
 * 
 * 1. All producers (via P2P signaling)
 * 2. All consumers (via C2C signaling)
 * 
 * Each producer or consumer allows another of its kind to proceed after
 * performing its operation.
 * The queue has a maximum size of 1000 elements.
 */
public class AllProducersAllConsumersV2 {

  // Shared synchronized queue
  public static MyQueue q = new MyQueue();

  // Shared lock and conditions
  private static final ReentrantLock lock = new ReentrantLock();

  // Conditions for controlling queue capacity
  private static final Condition isFull = lock.newCondition(); // Queue full — producers wait
  private static final Condition isEmpty = lock.newCondition(); // Queue empty — consumers wait

  // Conditions for intra-group cooperation
  private static final Condition P2P = lock.newCondition(); // Producer-to-producer signaling
  private static final Condition C2C = lock.newCondition(); // Consumer-to-consumer signaling

  public static void main(String[] args) {
    ExecutorService executor = Executors.newCachedThreadPool();

    // Launch multiple producers and consumers
    for (int i = 1; i < 10; i++) {
      executor.execute(new Producer());
      executor.execute(new Consumer());
    }

    executor.shutdown();

    // Wait for all threads to complete
    while (!executor.isTerminated()) {
      Thread.yield();
    }

    System.out.println("The size = " + q.size());
  }

  /**
   * Producer thread that adds timestamps to the queue.
   */
  static class Producer extends Thread {
    @Override
    public void run() {
      Date d = new Date();
      try {
        q.addEl(d);
      } catch (InterruptedException ex) {
        Logger.getLogger(AllProducersAllConsumersV2.class.getName()).log(Level.SEVERE, null, ex);
        Thread.currentThread().interrupt();
      }
    }
  }

  /**
   * Consumer thread that removes timestamps from the queue.
   */
  static class Consumer extends Thread {
    @Override
    public void run() {
      try {
        q.removeEl();
      } catch (InterruptedException ex) {
        Logger.getLogger(AllProducersAllConsumersV2.class.getName()).log(Level.SEVERE, null, ex);
        Thread.currentThread().interrupt();
      }
    }
  }

  /**
   * A thread-safe bounded queue that supports producer-to-producer and
   * consumer-to-consumer cooperation.
   */
  static class MyQueue extends LinkedList<Date> {

    /**
     * Adds an element to the queue, signaling other producers after insertion.
     * Waits if the queue is full.
     *
     * @param e the Date object to add
     * @throws InterruptedException if the thread is interrupted
     */
    public void addEl(Date e) throws InterruptedException {
      // Each producer adds 9 items
      for (int i = 1; i < 10; i++) {
        lock.lock();
        try {
          // Wait if the queue is full
          while (q.size() == 1000) {
            isFull.await();
          }

          // Add the element
          super.addLast(e);
          System.out.println(e + " is added by Producer " + Thread.currentThread().getId());

          // Notify consumers that the queue is no longer empty
          isEmpty.signal();

          // Let another producer continue
          P2P.signal();

          // Wait for the next turn among producers
          P2P.await();
        } finally {
          lock.unlock();
        }
      }
    }

    /**
     * Removes an element from the queue, signaling other consumers after removal.
     * Waits if the queue is empty.
     *
     * @throws InterruptedException if the thread is interrupted
     */
    public void removeEl() throws InterruptedException {
      // Each consumer removes 9 items
      for (int i = 1; i < 10; i++) {
        lock.lock();
        try {
          // Wait if the queue is empty
          while (q.size() == 0) {
            isEmpty.await();
          }

          // Remove the element
          Date d = super.removeFirst();
          System.out.println(d + " is removed by Consumer " + Thread.currentThread().getId());

          // Notify producers that the queue is no longer full
          isFull.signal();

          // Let another consumer continue
          C2C.signal();

          // Wait for the next turn among consumers
          C2C.await();
        } finally {
          lock.unlock();
        }
      }
    }
  }
}
