#!/bin/bash

#SBATCH --job-name=prefixsum
#SBATCH --partition=i64m512u
#SBATCH -N 1
#SBATCH --output=%j.out

lscpu

