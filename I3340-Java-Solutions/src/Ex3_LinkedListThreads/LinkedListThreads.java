package Ex3_LinkedListThreads;

import java.util.LinkedList;
import java.util.Random;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A multithreaded producer-consumer simulation using a shared LinkedList and condition variables.
 * Tasks are managed using an ExecutorService. The AddTask adds elements, while the RemoveTask removes them.
 */
public class LinkedListThreads {

    /**
     * Shared lock for synchronizing access to the linked list.
     */
    private static final Lock lock = new ReentrantLock();

    /**
     * Shared linked list used as the shared buffer.
     */
    private static final LinkedList<String> ll = new LinkedList<>();

    /**
     * Condition to signal that the list is not empty.
     */
    private static final Condition notEmpty = lock.newCondition();

    /**
     * Main method to simulate concurrent additions and removals to/from a shared list.
     *
     * @param args command-line arguments (not used)
     * @throws InterruptedException if any thread is interrupted
     */
    public static void main(String[] args) throws InterruptedException {

        System.out.println("------------------ Start OF MAIN ------------------------");

        // Thread pool to manage AddTask and RemoveTask threads dynamically
        ExecutorService executor = Executors.newCachedThreadPool();

        // Submit a mix of Add and Remove tasks
        for (int i = 0; i < 1000; i++) {
            executor.submit(new AddTask());
            executor.submit(new RemoveTask());
        }

        // Shutdown executor and wait for all tasks to complete
        executor.shutdown();
        while (!executor.isTerminated()) {
            System.out.println("Main is sleeping...yielding control");
            Thread.yield();
        }

        System.out.println("------------------ End OF MAIN ------------------------");
        System.out.println("The final size of the list = " + ll.size());
    }

    /**
     * A task that adds a random number (as string) to the shared linked list.
     * Signals any waiting RemoveTask that the list is no longer empty.
     */
    private static class AddTask extends Thread {

        @Override
        public void run() {
            lock.lock();
            try {
                int x = new Random().nextInt(100) + 1;
                ll.add(String.valueOf(x));
                System.out.println("An element was added: " + x);
                Thread.sleep(5); // Simulate slight delay
                notEmpty.signal(); // Wake up one waiting RemoveTask
            } catch (InterruptedException ex) {
                Logger.getLogger(LinkedListThreads.class.getName()).log(Level.SEVERE, null, ex);
                System.out.println("---------- AddTask was interrupted ----------");
            } finally {
                lock.unlock();
            }
        }
    }

    /**
     * A task that removes an element from the shared linked list.
     * Waits if the list is empty until an AddTask signals it.
     */
    private static class RemoveTask extends Thread {

        @Override
        public void run() {
            lock.lock();
            try {
                while (ll.isEmpty()) {
                    notEmpty.await(); // Wait until notified that the list is not empty
                }
                String x = ll.remove();
                System.out.println("An element was removed: " + x);
            } catch (InterruptedException ex) {
                Logger.getLogger(LinkedListThreads.class.getName()).log(Level.WARNING, "RemoveTask was interrupted", ex);
            } finally {
                lock.unlock();
            }
        }
    }
}
