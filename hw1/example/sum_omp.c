/*
 * sum_omp.c
 *
 * Description: Parallel implementation of Sum reduction program to sum a
 * sequence of randomly generated integers using OpenMP.
 *
 * Procedure:
 * 1. All the threads generate num_elems random integers (in parallel OpenMP
 *    region);
 * 2. All the threads compute the final sum of the inputs in parallel
 *    through shared memory (in parallel OpenMP region).
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
//#define PRINT_SUM

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
    long sum;

    struct timeval start_time, end_time;  // for gettimeofday to calculate timing

    char filename[256] = "sum_omp_";
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
    if (data == NULL) {
        printf("Failed in malloc()\n");
        printf(" - data: %p\n", data);
        free(data);
        exit(-2);
    }
    memset(data, 0, num_elems);

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

    // Compute the sum reduction in parallel
    printf("Start ...\n");
    fprintf(fp, "Start ...\n");

    suseconds_t iter_usec = 0;
    suseconds_t total_usec = 0;
    int iter;
    int i;
    for (iter = 0; iter < num_iters; iter++) {

        sum = 0;

        gettimeofday(&start_time, NULL);
        // Parallel sum reduction using OpenMP
        #pragma omp parallel for reduction(+:sum)
        for (i = 0; i < num_elems; i++) {
            sum += data[i];
        }
        gettimeofday(&end_time, NULL);

        iter_usec = usec(start_time, end_time);
        total_usec += iter_usec;

        printf("iteration %d elapsed time: %d (usec)\n", iter, iter_usec);
        fprintf(fp, "iteration %d elapsed time: %d (usec)\n", iter, iter_usec);
    }

    // Print timing stats
    printf("Finish OpenMP Parrallel Sum calculation\n\n");
    fprintf(fp, "Finish OpenMP Parrallel Sum calculation\n\n");
    printf("Sum average elapsed time: %d (usec)\n", total_usec / num_iters);
    fprintf(fp, "Sum average elapsed time: %d (usec)\n",
            total_usec / num_iters);

#ifdef PRINT_SUM
    fprintf(fp, "\nInputs:");
    for (i = 0; i < num_elems; i++) {
        fprintf(fp, " %d:%d", i, data[i]);
    }
    fprintf(fp, "\n\nSum: %ld\n", sum);
#endif // #ifdef PRINT_SUM

    // free the allocated memory
    free(starts);
    free(ends);
    free(data);

    fclose(fp);

    return 0;
}
