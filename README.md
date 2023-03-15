# OS_Homework2 Multi-threaded and kernel module programming

1. A multithreaded program to perform matrix multiplication.
2. The program starts with a single main/parent thread, which is responsible for creating multiple worker threads.
3. Each worker thread should perform a part of the matrix multiplication job.
4. Each worker thread should write its thread ID to the proc file right before its termination.
5. After completing the matrix multiplication, the main thread has to read the proc file and print the resulting information (context switch times and execution time) on the console.
6. After completing the matrix multiplication, the program also has to save the result of the matrix multiplication (i.e. the result matrix) to a file named as result.txt.
7. The executable and parameters of your multithread program: ./MT_matrix [number of worker threads] [file name of input matrix1] [file name of input matrix2]
