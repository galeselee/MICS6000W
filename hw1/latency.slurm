#!/bin/bash

#SBATCH --job-name=prefixsum
#SBATCH --partition=i64m512u
#SBATCH -N 2
#SBATCH -n 2
#SBATCH --output=%j.out

module load mpi/mpich-4.1.2
rm machinefile
bash machinefile.sh
mpirun -np 2 -machinefile machinefile ./latency
