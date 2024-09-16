CC     = gcc     # the c compiler to use
MPICC  = mpicc   # the MPI cc compiler
CFLAGS = -O3     # optimize code
DFLAGS =         # common defines
LIB    = -lm     # link libraries

default: all

all: prefixsum_omp.exe prefixsum_mpi.exe prefixsum_seq.exe

prefixsum_omp.exe: prefixsum_omp.c
	$(CC) $(CFLAGS) $(DFLAGS) $(LIB) -fopenmp -o $@ $<

prefixsum_mpi.exe: prefixsum_mpi.c
	$(MPICC) $(CFLAGS) $(DFLAGS) $(LIB) -o $@ $<

prefixsum_seq.exe: prefixsum_seq.c
	$(CC) $(CFLAGS) $(DFLAGS) $(LIB) -o $@ $<

clean:
	rm *.exe
