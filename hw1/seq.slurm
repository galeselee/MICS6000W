#!/bin/bash

#SBATCH --job-name=prefixsum
#SBATCH --partition=i64m512u
#SBATCH -N 1
#SBTACH -n 64
#SBATCH --output=%j.out


./prefixsum_seq.exe 100000000 10
