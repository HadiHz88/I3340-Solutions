package Ex1_CountingTwoWords;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

/**
 * A multithreaded Java program that counts the number of occurrences of the
 * word "Alice"
 * in two separate text files using two threads.
 */
public class CountingTwoWords {

    /**
     * Main entry point. Initializes two threads to count occurrences of a word in
     * separate files.
     *
     * @param args command-line arguments (not used)
     */
    public static void main(String[] args) {

        // Replace with the absolute path to the files if necessary
        File f1 = new File("alice-chap1-6.txt");
        File f2 = new File("alice-chap7-12.txt");

        CountingTask task1 = new CountingTask(f1, "Alice");
        CountingTask task2 = new CountingTask(f2, "Alice");

        task1.start();
        task2.start();

        // Busy-wait until both threads are done
        while (task1.isAlive() || task2.isAlive()) {
            Thread.yield();
        }

        // Calculate total count from both threads
        int total = task1.getCounter() + task2.getCounter();
        System.out.println("Total count of Alice: " + total);
    }

    /**
     * A thread class responsible for counting the number of times a specific word
     * appears in a given file.
     */
    static class CountingTask extends Thread {

        private final File file;
        private final String word;
        private int counter = 0;

        /**
         * Constructs a CountingTask with the specified file and target word.
         *
         * @param file the file to be scanned
         * @param word the word to count within the file
         */
        public CountingTask(File file, String word) {
            this.file = file;
            this.word = word;
        }

        /**
         * Returns the number of times the word was found in the file.
         *
         * @return the word count
         */
        public int getCounter() {
            return counter;
        }

        /**
         * The run method executed when the thread starts.
         * It reads the file line by line and counts occurrences of the target word.
         */
        @Override
        public void run() {
            FileReader fr = null;

            try {
                fr = new FileReader(this.file);
            } catch (FileNotFoundException ex) {
                Logger.getLogger(CountingTwoWords.class.getName()).log(Level.SEVERE, null, ex);
            }

            assert fr != null;

            BufferedReader br = new BufferedReader(fr);

            String line;
            System.out.println("Searching in file: " + file.getName());

            try {
                while ((line = br.readLine()) != null) {
                    String[] words = line.split(" ");
                    for (String currentWord : words) {
                        if (currentWord.equals(word)) {
                            counter++;
                        }
                    }
                }
            } catch (IOException ex) {
                Logger.getLogger(CountingTwoWords.class.getName()).log(Level.SEVERE, null, ex);
            }

            try {
                fr.close();
            } catch (IOException ex) {
                Logger.getLogger(CountingTwoWords.class.getName()).log(Level.SEVERE, null, ex);
            }

            System.out.println("Count in file " + file.getName() + ": " + this.getCounter());
        }
    }
}
