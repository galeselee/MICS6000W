CC     = gcc     # the c compiler to use
MPICC  = mpicc   # the MPI cc compiler
CFLAGS = -O3     # optimize code
DFLAGS =         # common defines
LIB    = -lm     # link libraries
MPIH = -I/opt/mpi/mpich-4.1.2/include/

default: all

all: prefixsum_seq.exe prefixsum_omp.exe prefixsum_mpi.exe

prefixsum_mpi.exe: prefixsum_mpi.c
	$(MPICC) $(CFLAGS) $(MPIH) $(DFLAGS) $(LIB) -o $@ $<

prefixsum_seq.exe: prefixsum_seq.c
	$(CC) $(CFLAGS) $(DFLAGS) $(LIB) -o $@ $<

prefixsum_omp.exe: prefixsum_omp.c
	$(CC) $(CFLAGS) $(DFLAGS) $(LIB) -fopenmp -o $@ $<

clean:
	rm *.exe
