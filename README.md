# I3340 - Parallel Programming

## Lebanese University 2024-2025

This repository contains my solutions for the Parallel Programming (I3340) course labs and exercises. The implementations cover various parallel programming paradigms and technologies.

## Repository Structure

### Java Multithreading Solutions

Located in `Java/src/`:

- **Ex1_CountingTwoWords**: Word counting across multiple files using multiple threads
- **Ex2_CounterThreads**: Thread synchronization with counters
- **Ex3_LinkedListThreads**: Thread-safe operations on linked lists
- **Ex4_StackThreads**: Concurrent stack implementations
- **Ex5_AllProducersAllConsumers**: Two variations of an all-producers-all-consumers pattern
- **Ex6_ProducerConsumer**: Classic producer-consumer problem implementation
- Other...

### POSIX Threads (PThreads) Solutions

Located in `Pthread/`:

- Multiple producer-consumer pattern implementations (pc_case_1.c through pc_case_6.c)
- Factorial, minimum, and maximum calculations (fact_min_max_1.c, fact_min_max_2.c)
- Other...

### OpenMP Solutions

Located in `OpenMP/` (currently in development)

### Previous Exams

Located in `Exams/` (currently in development)

## Getting Started

### Java Solutions

```bash
cd Java
# Compile and run individual exercises, for example:
javac src/Ex1_CountingTwoWords/CountingTwoWords.java
java -cp src Ex1_CountingTwoWords.CountingTwoWords
```

### PThread Solutions

```bash
cd Pthread
# Compile and run individual exercises, for example:
gcc -o pc_case_1 pc_case_1.c -pthread
./pc_case_1
```

## Learning Objectives

This course covers the following parallel programming concepts:

- Thread creation and synchronization
- Mutex and condition variables
- Producer-consumer patterns
- Thread-safe data structures
- Parallel algorithms
- Performance optimization techniques

## Technologies

- Java Multithreading
- POSIX Threads (PThreads)
- OpenMP (Coming soon)
- MPI (Coming soon)

## References

- Course materials and assignments provided by Dr. Kamal Baydoun- Lebanese University
