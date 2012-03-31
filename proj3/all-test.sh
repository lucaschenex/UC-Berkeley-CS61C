#!/bin/bash
#PBS -N CS61C
#PBS -V
#PBS -l nodes=1
#PBS -q batch
cd $PBS_O_WORKDIR
# workaround to fix a thread affinity problem in Linux
export GOTOBLAS_MAIN_FREE=1
# Change OMP_NUM_THREADS to set number of threads
export OMP_NUM_THREADS=8
export GOMP_CPU_AFFINITY="0-7"
# name of the file to execute
./bench-all
