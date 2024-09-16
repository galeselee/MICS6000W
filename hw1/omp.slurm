#!/bin/bash

#SBATCH --job-name=prefixsum
#SBATCH --partition=i64m512u
#SBATCH -N 1
#SBATCH -n 64
#SBATCH --output=%j.out


taskset -c 4-7,36-39 ./prefixsum_omp.exe 100000000 10 8
