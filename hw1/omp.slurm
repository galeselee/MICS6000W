#!/bin/bash

#SBATCH --job-name=prefixsum
#SBATCH --partition=i64m512u
#SBATCH -N 1
#SBATCH -n 64
#SBATCH --output=%j.out
#SBATCH --error=%j.err


taskset -c 0,1 ./prefixsum_omp.exe 100000000 10 2
