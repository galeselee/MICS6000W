/*
 * prefixsum_seq.c
 *
 * Description: Sequential implementation of Prefix Sum program to sum a
 * sequence of randomly generated integers.
 *
 * Procedure:
 * 1. The processor generates num_elems random integers;
 * 2. The processor compute the prefix sums from the first element to the last
 *    one. Next prefix sum equals to the sum of its corresponding integer and
 *    the previous prefix sum. The computation complexity is O(N).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/time.h>

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
    int num_elems = 0;
    int num_iters = 0;

    int *data = NULL;
    long *prefix_sums = NULL;

    struct timeval start, end;  // for gettimeofday to calculate timing

    char filename[256] = "prefixsum_seq_";
    FILE *fp = NULL;

    if (argc < 3) {
        printf("Usage: %s [num_elems] [num_iters]\n", argv[0]);
        printf("    - num_elems:  number of elements\n");
        printf("    - num_iters: number of iterations\n");
        exit(-1);
    }

    num_elems = atoi(argv[1]);
    num_iters = atoi(argv[2]);

    strcat(filename, argv[1]);
    strcat(filename, "elems_");
    strcat(filename, argv[2]);
    strcat(filename, "iters.txt");

    fp = fopen(filename, "w");
    if (fp) {
        printf("Command line: %s %d %d\n", argv[0], num_elems, num_iters);
        printf("Stats file: %s\n\n", filename);
        fprintf(fp, "Command line: %s %d %d\n", argv[0], num_elems, num_iters);
        fprintf(fp, "Stats file: %s\n\n", filename);
    } else {
        printf("ERROR: can't open the file %s!\n", filename);
        exit(-1);
    }

    // Memory allocation
    data = (int *) malloc(sizeof(int) * num_elems);
    memset(data, 0, num_elems);
    prefix_sums = (long *) malloc(sizeof(long) * num_elems);
    memset(prefix_sums, 0, num_elems);

    // Generate random ints sequentially
    int K = MAX_INT / num_elems;

    srand(time(NULL));  // Seed rand function

    int i;
    for (i = 0; i < num_elems; i++) {
        data[i] = rand() % K;
    }

    // Compute the prefix sums sequentially
    printf("Start ...\n");
    fprintf(fp, "Start ...\n");

    suseconds_t iter_usec = 0;
    suseconds_t total_usec = 0;
    int iter;
    for (iter = 0; iter < num_iters; iter++) {

        for (i = 0; i < num_elems; i++) {
            prefix_sums[i] = data[i];
        }

        gettimeofday(&start, NULL);
        /************************************************************/
        /* PLEASE COMPLETE THE CODE - Begin                         */
        /************************************************************/

        /************************************************************/
        /* PLEASE COMPLETE THE CODE - End                           */
        /************************************************************/
        gettimeofday(&end, NULL);

        iter_usec = usec(start, end);
        total_usec += iter_usec;

        printf("iteration %d elapsed time: %d (usec)\n", iter, iter_usec);
        fprintf(fp, "iteration %d elapsed time: %d (usec)\n", iter, iter_usec);
    }

    // Print timing stats
    printf("Finish Prefix Sum calculation\n\n");
    fprintf(fp, "Finish Prefix Sum calculation\n\n");
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

    // free the allocated memory
    free(data);
    free(prefix_sums);

    fclose(fp);

    return 0;
}
