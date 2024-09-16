// mpi.slurm.test
// mpicc prefixsum_mpi_test.c -o mpi_test -DPRINT_PREFIXSUM -lm -O3
// mpirun -n 8 ./mpi_test 80 1

/*
 * prefixsum_mpi.c
 *
 * Description: Parallel implementation of Prefix Sum program to sum a
 * sequence of randomly generated integers using MPI.
 *
 * Procedure:
 * 1. Each processor generates num_elems random integers in parallel;
 * 2. Each processor computes its local prefix sums if it has more than 1
 *    element in parallel.
 * 3. All the processors run in parallel to compute the prefix sum: Each
 *    processor sets its local largest prefix sum to a buffer, and receives a
 *    message from other processor that contains the sum of previous data in a
 *    binary tree manner. Lastly, each processor has the sum of all the
 *    previous data.
 * 4. Finally, each processor uses the sum of all the previous data to update
 *    the local prefix sum to get the final result.
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
//#define PRINT_PREFIXSUM

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
    long *local_prefix_sums = NULL;
    int *tmp_sums = NULL;
    long *buffer = NULL;
    long *buffer_update = NULL;

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

    char filename[256] = "prefixsum_mpi_";
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
            free(local_prefix_sums);
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
        // my_num_elems = num_elems;
        my_num_elems = num_elems_mean;
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
    local_prefix_sums = (long *) malloc(sizeof(long) * my_num_elems);
    buffer = (long *) malloc(sizeof(long));
    buffer_update = (long *) malloc(sizeof(long));
    if (local_data == NULL || local_prefix_sums == NULL || buffer == NULL) {
        printf("Processor %d failed in malloc.\n", rank);
        printf(" - local_data: %p\n", local_data);
        printf(" - local_prefix_sums: %p\n", local_prefix_sums);
        printf(" - buffer: %p\n", buffer);
        free(local_data);
        free(local_prefix_sums);
        free(buffer);
        MPI_Finalize();
        exit(-2);
    }

    // generate input data
    srand(rank + time(NULL));
    int i;
    int K = MAX_INT / num_elems;
    for (i = 0; i < my_num_elems; i++) {
        // local_data[i] = rand() % K;
         local_data[i] = 1;
    }

    MPI_Barrier(MPI_COMM_WORLD);    // Global barrier

    // Compute the local prefix sums in each process in parallel
    if (rank == 0) {
        printf("Start ...\n");
        fprintf(fp, "Start ...\n");
    }

    suseconds_t iter_usec = 0;
    suseconds_t total_usec = 0;

    int iter;
    for (iter = 0; iter < num_iters; iter++) {
        // copy the input array to the prefix sum array for initialization
        for (i = 0; i < my_num_elems; i++) {
            local_prefix_sums[i] = local_data[i];
        }

        gettimeofday(&start_time, NULL);
        /************************************************************/
        /* PLEASE COMPLETE THE CODE - Begin                         */
        /************************************************************/

        for (int ii = 1; ii < my_num_elems; ii++) {
            local_prefix_sums[ii] += local_prefix_sums[ii-1];
        }
        if (rank != 0) {
            MPI_Recv(buffer, 1, MPI_LONG, rank - 1, 0, MPI_COMM_WORLD, &status);
            buffer_update[0] = local_prefix_sums[my_num_elems - 1] + buffer[0];
        } else {
            buffer_update[0] = local_prefix_sums[my_num_elems - 1];
            buffer[0] = 0;
        }

        if (rank != num_procs - 1) {
            MPI_Send(buffer_update, 1, MPI_LONG, rank + 1, 0, MPI_COMM_WORLD);
        }
        for (int ii = 0; ii < my_num_elems; ii++) {
            local_prefix_sums[ii] += buffer[0];
        }

        /************************************************************/
        /* PLEASE COMPLETE THE CODE - End                           */
        /************************************************************/
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
        printf("Finish MPI Parallel Prefix Sum calculation\n\n");
        fprintf(fp, "Finish MPI Parallel Prefix Sum calculation\n\n");
        printf("Prefix Sum average elapsed time: %d (usec)\n",
                total_usec / num_iters);
        fprintf(fp, "Prefix Sum average elapsed time: %d (usec)\n",
                total_usec / num_iters);

#ifdef PRINT_PREFIXSUM
        fprintf(fp, "\nInputs:");
#endif // #ifdef PRINT_PREFIXSUM
        fclose(fp);
    } else {
#ifdef PRINT_PREFIXSUM
        // this is used to syncrhonize the processes so that only one process
        // writes to the file at a time
        MPI_Recv(buffer, 1, MPI_LONG, rank-1, 0, MPI_COMM_WORLD, &status);
#endif // #ifdef PRINT_PREFIXSUM
    }

#ifdef PRINT_PREFIXSUM
    // print the input and computed results
    fp = fopen(filename, "a");
    if (fp == NULL) {
        printf("ERROR: Processor %d failed in openning the file %s!\n",
                rank, filename);
        free(local_data);
        free(local_prefix_sums);
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

    // wait and receive the turn to write the prefix sums
    int src = (rank + num_procs - 1) % num_procs;
    MPI_Recv(buffer, 1, MPI_LONG, src, 0, MPI_COMM_WORLD, &status);

    fp = fopen(filename, "a");
    if (fp == NULL) {
        printf("ERROR: Processor %d failed in openning the file %s!\n",
                rank, filename);
        free(local_data);
        free(local_prefix_sums);
        free(buffer);
        MPI_Finalize();
        exit(-1);
    }

    if (rank == 0) {
        fprintf(fp, "\n\nPrefix Sums:");
    }
    for (i = 0; i < my_num_elems; i++) {
        fprintf(fp, " %d:%d", start+i, local_prefix_sums[i]);
    }

    // finish prefix sum  write, pass to the next processor (syncrhonization)
    if (rank == num_procs - 1) {
        // last writer, finish up
        fprintf(fp, "\n");
        fclose(fp);
    } else {
        fclose(fp);
        MPI_Send(buffer, 1, MPI_LONG, dest, 0, MPI_COMM_WORLD);
    }
#endif // #ifdef PRINT_PREFIXSUM

    free(local_data);
    free(local_prefix_sums);
    free(buffer);

    MPI_Finalize();

    return 0;
}
