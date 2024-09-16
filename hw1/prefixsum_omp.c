/*
 * prefixsum_omp.c
 *
 * Description: Parallel implementation of Prefix Sum program to sum a
 * sequence of randomly generated integers using OpenMP.
 *
 * Procedure:
 * 1. All the threads generate num_elems random integers (in parallel OpenMP
 *    region);
 * 2. Each thread computes its corresponding the prefix sums if its data
 *    partition has more than 1 element (in parallel OpenMP region).
 * 3. All the threads take the largest local prefix sum, the last one in its
 *    corresponding partition, to make a new temporary data array. Then, the
 *    threads compute the prefix sum of the the temporary array in parallel
 *    through shared memory. After computation, each temporary array element
 *    has the sum of the previous input data and its local largest prefix sum.
 * 4. Each thread updates the local prefix sums using the corresponding
 *    temporary array element (in parallel OpenMP region).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <omp.h>

#define MAX_INT 2147483647
//#define PRINT_PREFIXSUM
#define VERIFY

// usec: calculate the time interval in microseconds
inline suseconds_t usec(struct timeval start, struct timeval end)
{
  return ((double) (((end.tv_sec * 1000000 + end.tv_usec) -
                     (start.tv_sec * 1000000 + start.tv_usec))));
}

int main(int argc, char *argv[])
{
    int num_elems = 0;
    int num_iters = 0;
    int num_threads = 0;

    int *data = NULL;
    long *prefix_sums = NULL;
    long *tmp_sums = NULL;

    struct timeval start_time, end_time;  // for gettimeofday to calculate timing

    char filename[256] = "prefixsum_omp_";
    FILE *fp = NULL;

    if (argc < 4) {
        printf("Usage: %s [num_elems] [num_iters] [num_threads]\n", argv[0]);
        printf("    - num_elems:  number of elements\n");
        printf("    - num_iters: number of iterations\n");
        printf("    - num_threads: number of threads\n");
        exit(-1);
    }

    num_elems = atoi(argv[1]);
    num_iters = atoi(argv[2]);
    num_threads = atoi(argv[3]);

    if (num_threads < 1) {
        printf("Number of threads should be more than one!\n");
        exit(-1);
    }

    strcat(filename, argv[1]);
    strcat(filename, "elems_");
    strcat(filename, argv[2]);
    strcat(filename, "iters_");
    strcat(filename, argv[3]);
    strcat(filename, "threads.txt");

    fp = fopen(filename, "w");
    if (fp) {
        printf("Command line: %s %d %d %d\n",
                argv[0], num_elems, num_iters, num_threads);
        printf("Stats file: %s\n\n", filename);
        fprintf(fp, "Command line: %s %d %d %d\n",
                argv[0], num_elems, num_iters, num_threads);
        fprintf(fp, "Stats file: %s\n\n", filename);
    } else {
        printf("ERROR: can't open the file %s!\n", filename);
        exit(-1);
    }

    // data partition and allocation
    int num_elems_mean = num_elems / num_threads;
    int num_elems_remain = num_elems % num_threads;
    // starting and ending IDs of data partition for each thread
    int *starts;
    int *ends;
    starts = (int *) malloc(sizeof(int) * num_threads);
    ends = (int *) malloc(sizeof(int) * num_threads);
    int id;
    for (id = 0; id < num_threads; id++) {
        if (num_elems_remain == 0) {
            starts[id] = id * num_elems_mean;
            ends[id] = starts[id] + num_elems_mean;
        } else {
            if (id < num_elems_remain) {
                starts[id] = id * (num_elems_mean + 1);
                ends[id] = starts[id] + (num_elems_mean + 1);
            } else {
                starts[id] = id * num_elems_mean + num_elems_remain;
                ends[id] = starts[id] + num_elems_mean;
            }
        }
    }

    // Memory allocation
    data = (int *) malloc(sizeof(int) * num_elems);
    prefix_sums = (long *) malloc(sizeof(long) * num_elems);
    tmp_sums = (long *) malloc(sizeof(long) * num_threads);
    if (data == NULL || prefix_sums == NULL || tmp_sums == NULL) {
        printf("Failed in malloc()\n");
        printf(" - data: %p\n", data);
        printf(" - prefix_sums: %p\n", prefix_sums);
        printf(" - tmp_sums: %p\n", tmp_sums);
        free(data);
        free(prefix_sums);
        free(tmp_sums);
        exit(-2);
    }
    memset(data, 0, num_elems);
    memset(prefix_sums, 0, num_elems);
    memset(tmp_sums, 0, num_threads);

    // set number of threads
    omp_set_num_threads(num_threads);

    // Generate random ints in parallel
    int K = MAX_INT / num_elems;

    #pragma omp parallel shared(starts, ends, K, data)
    {
        // get the local thread ID
        int tid = omp_get_thread_num();
        srand(tid + time(NULL));  // Seed rand function

        int start = starts[tid];
        int end = ends[tid];

        int i;
        for (i = start; i < end; i++) {
            data[i] = rand() % K;
        }
    }

    // Compute the prefix sums in each thread in parallel, where each thread
    // sequentially computes the local prefix sums
    printf("Start ...\n");
    fprintf(fp, "Start ...\n");

    suseconds_t iter_usec = 0;
    suseconds_t total_usec = 0;
    int iter;
    for (iter = 0; iter < num_iters; iter++) {
        // copy the input array to the prefix sum array for initialization
        #pragma omp parallel shared(starts, ends, data, prefix_sums)
        {
            int tid = omp_get_thread_num(); // get the local thread ID

            int start = starts[tid];
            int end = ends[tid];

            int i;
            for (i = start; i < end; i++)
                prefix_sums[i] = data[i];
        }

        gettimeofday(&start_time, NULL);
        /************************************************************/
        /* PLEASE COMPLETE THE CODE - Begin                         */
        /************************************************************/

        /************************************************************/
        /* PLEASE COMPLETE THE CODE - End                           */
        /************************************************************/
        gettimeofday(&end_time, NULL);

        iter_usec = usec(start_time, end_time);
        total_usec += iter_usec;

        printf("iteration %d elapsed time: %d (usec)\n", iter, iter_usec);
        fprintf(fp, "iteration %d elapsed time: %d (usec)\n", iter, iter_usec);
    }

    // Print timing stats
    printf("Finish OpenMP Parrallel Prefix Sum calculation\n\n");
    fprintf(fp, "Finish OpenMP Parrallel Prefix Sum calculation\n\n");
    printf("Prefix Sum average elapsed time: %d (usec)\n",
            total_usec / num_iters);
    fprintf(fp, "Prefix Sum average elapsed time: %d (usec)\n",
            total_usec / num_iters);

#ifdef PRINT_PREFIXSUM
    fprintf(fp, "\nInputs:");
    for (i = 0; i < num_elems; i++) {
        fprintf(fp, " %d:%d", i, data[i]);
    }
    fprintf(fp, "\n\nPrefix Sums:");
    for (i = 0; i < num_elems; i++) {
        fprintf(fp, " %d:%ld", i, prefix_sums[i]);
    }
    fprintf(fp, "\n");
#endif // #ifdef PRINT_PREFIXSUM

#ifdef VERIFY
    long *verify_prefix_sums = malloc(sizeof(long) * num_elems);
    memset(verify_prefix_sums, 0, num_elems);
    verify_prefix_sums[0] = data[0];
    int i;
    for (i = 1; i < num_elems; i++) {
        verify_prefix_sums[i] = data[i] + verify_prefix_sums[i-1];
        if (verify_prefix_sums[i] != prefix_sums[i]) {
            printf("Wrong parallel prefix sum implementation: error at position %d, true prefix sum: %ld, computed prefix sum: %ld\n",
                    i, verify_prefix_sums[i], prefix_sums[i]);
            exit(-1);
        }
    }
#endif // #ifdef VERIFY

    // free the allocated memory
    free(starts);
    free(ends);
    free(data);
    free(prefix_sums);
    free(tmp_sums);

    fclose(fp);

    return 0;
}
