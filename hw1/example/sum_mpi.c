/*
 * sum_mpi.c
 *
 * Description: Parallel implementation of Sum Reduction program to sum a
 * sequence of randomly generated integers using MPI.
 *
 * Procedure:
 * 1. Each processor generates num_elems random integers in parallel;
 * 2. All the processors run in parallel to compute the final sum using MPI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <mpi.h>

#define MAX_INT 2147483647
//#define PRINT_SUM

// usec: calculate the time interval in microseconds
inline suseconds_t usec(struct timeval start, struct timeval end)
{
  return ((double) (((end.tv_sec * 1000000 + end.tv_usec) -
                     (start.tv_sec * 1000000 + start.tv_usec))));
}

int main(int argc, char *argv[])
{
    // command line arguments
    int num_elems = 0;
    int num_iters = 0;
    int num_procs = 0;

    int rank;

    // per-processor local memory pointers
    int *local_data = NULL;
    long local_sum;
    long sum;
    long *buffer = NULL;

    struct timeval start_time, end_time;  // for gettimeofday to calculate timing

    MPI_Status status; // status variable for MPI operations

    // Initialize MPI environment
    // - num_procs instances of this program will be initiated by MPI.
    // - All the variables will be local/private to each process, only the
    //   program owner could see its own variables.
    // - If there must be a inter-processor communication, it must be done
    //   explicitly using MPI communication functions.

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // getting the ID for this process

    if (argc < 3) {
        if (rank == 0) {
            printf("Usage: %s [num_elems] [num_iters]\n", argv[0]);
            printf("    - num_elems:  number of elements\n");
            printf("    - num_iters: number of iterations\n");
        }

        MPI_Finalize();
        exit(-1);
    }

    num_elems = atoi(argv[1]);
    num_iters = atoi(argv[2]);

    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // get the number of processes

    char filename[256] = "sum_mpi_";
    char nprocs[5];
    sprintf(nprocs, "%d", num_procs);
    FILE *fp = NULL;

    strcat(filename, argv[1]);
    strcat(filename, "elems_");
    strcat(filename, argv[2]);
    strcat(filename, "iters_");
    strcat(filename, nprocs);
    strcat(filename, "procs.txt");

    if (rank == 0) {
        fp = fopen(filename, "w");
        if (fp) {
            printf("Command line: mpirun -np %d %s %d %d\n",
                    num_procs, argv[0], num_elems, num_iters);
            printf("Stats file: %s\n\n", filename);
            fprintf(fp, "Command line: mpirun -np %d %s %d %d\n",
                    num_procs, argv[0], num_elems, num_iters);
            fprintf(fp, "Stats file: %s\n\n", filename);
        } else {
            printf("ERROR: can't open the file %s!\n", filename);
            free(local_data);
            free(buffer);
            MPI_Finalize();
            exit(-1);
        }
    }

    // data patition varies due to the input data size
    int my_num_elems;
    int num_elems_mean = num_elems / num_procs;
    int num_elems_remain = num_elems % num_procs;
    if (rank < num_elems_remain) {
        my_num_elems = num_elems_mean + 1;
    } else {
        my_num_elems = num_elems;
    }

    int start, end;
    if (num_elems_remain == 0) {
        start = rank * num_elems_mean;
        end = start + num_elems_mean;
    } else {
        if (rank < num_elems_remain) {
            start = rank * (num_elems_mean + 1);
            end = start + (num_elems_mean + 1);
        } else {
            start = rank * num_elems_mean + num_elems_remain;
            end = start + num_elems_mean;
        }
    }

    // Memory allocation private to each process
    local_data = (int *) malloc(sizeof(int) * my_num_elems);
    buffer = (long *) malloc(sizeof(long));
    if (local_data == NULL || buffer == NULL) {
        printf("Processor %d failed in malloc.\n", rank);
        printf(" - local_data: %p\n", local_data);
        printf(" - buffer: %p\n", buffer);
        free(local_data);
        free(buffer);
        MPI_Finalize();
        exit(-2);
    }

    // generate input data
    srand(rank + time(NULL));
    int i;
    int K = MAX_INT / num_elems;
    for (i = 0; i < my_num_elems; i++) {
        local_data[i] = rand() % K;
    }

    MPI_Barrier(MPI_COMM_WORLD);    // Global barrier

    // Compute the sum reductin in each process in parallel using MPI
    if (rank == 0) {
        printf("Start ...\n");
        fprintf(fp, "Start ...\n");
    }

    suseconds_t iter_usec = 0;
    suseconds_t total_usec = 0;

    int iter;
    for (iter = 0; iter < num_iters; iter++) {

        gettimeofday(&start_time, NULL);

        // compute the local sum in each process
        local_sum = 0;
        sum = 0;
        for (i = 0; i < my_num_elems; i++) {
            local_sum += local_data[i];
        }

        MPI_Reduce(&local_sum, &sum, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

        gettimeofday(&end_time, NULL);

        iter_usec = usec(start_time, end_time);
        total_usec += iter_usec;

        if (rank == 0) {
            printf("iteration %d elapsed time: %d (usec)\n", iter, iter_usec);
            fprintf(fp, "iteration %d elapsed time: %d (usec)\n",
                    iter, iter_usec);
        }
    }

    // print timing stats
    if (rank == 0) {
        printf("Finish MPI Parallel Sum calculation\n\n");
        fprintf(fp, "Finish MPI Parallel Sum calculation\n\n");
        printf("Sum average elapsed time: %d (usec)\n",
                total_usec / num_iters);
        fprintf(fp, "Sum average elapsed time: %d (usec)\n",
                total_usec / num_iters);

#ifdef PRINT_SUM
        fprintf(fp, "\nInputs:");
#endif // #ifdef PRINT_SUM
        fclose(fp);
    } else {
#ifdef PRINT_SUM
        // this is used to syncrhonize the processes so that only one process
        // writes to the file at a time
        MPI_Recv(buffer, 1, MPI_LONG, rank-1, 0, MPI_COMM_WORLD, &status);
#endif // #ifdef PRINT_SUM
    }

#ifdef PRINT_SUM
    // print the input and computed results
    fp = fopen(filename, "a");
    if (fp == NULL) {
        printf("ERROR: Processor %d failed in openning the file %s!\n",
                rank, filename);
        free(local_data);
        free(buffer);
        MPI_Finalize();
        exit(-1);
    }

    for (i = 0; i < my_num_elems; i++) {
        fprintf(fp, " %d:%d", start+i, local_data[i]);
    }

    fclose(fp);

    // finish input write, pass to the next processor (syncrhonization)
    int dest = (rank + 1) % num_procs;
    MPI_Send(buffer, 1, MPI_LONG, dest, 0, MPI_COMM_WORLD);

    // process 0 wait and receive the turn to write the sum
    if (rank == 0) {
        int src = num_procs - 1;
        MPI_Recv(buffer, 1, MPI_LONG, src, 0, MPI_COMM_WORLD, &status);

        fp = fopen(filename, "a");
        if (fp == NULL) {
            printf("ERROR: Processor %d failed in openning the file %s!\n",
                    rank, filename);
            free(local_data);
            free(buffer);
            MPI_Finalize();
            exit(-1);
        }

        fprintf(fp, "\n\nSum: %ld\n", sum);
        fclose(fp);
    }
#endif // #ifdef PRINT_SUM

    free(local_data);
    free(buffer);

    MPI_Finalize();

    return 0;
}
