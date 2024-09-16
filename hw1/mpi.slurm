#!/bin/bash

#SBATCH --job-name=prefixsum
#SBATCH --partition=i64m512u
#SBATCH -N 2
#SBATCH -n 64
#SBATCH --output=%j.out

module load mpi/mpich-4.1.2

rm machinefile
bash machinefile.sh
mpirun -n 64 -machinefile machinefile taskset -c 0-15,32-47 ./prefixsum_mpi.exe 640000000 10
