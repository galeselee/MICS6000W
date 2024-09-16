CC     = gcc     # the c compiler to use
MPICC  = mpicc   # the MPI cc compiler
CFLAGS = -O3     # optimize code
DFLAGS =         # common defines
LIB    = -lm     # link libraries

default: all

all: sum_omp.exe sum_mpi.exe sum_seq.exe

sum_omp.exe: sum_omp.c
	$(CC) $(CFLAGS) $(DFLAGS) $(LIB) -fopenmp -o $@ $<

sum_mpi.exe: sum_mpi.c
	$(MPICC) $(CFLAGS) $(DFLAGS) $(LIB) -o $@ $<

sum_seq.exe: sum_seq.c
	$(CC) $(CFLAGS) $(DFLAGS) $(LIB) -o $@ $<

clean:
	rm *.exe
