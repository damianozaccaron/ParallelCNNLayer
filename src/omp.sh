#!/bin/bash

#PBS -l select=1:ncpus=2:mem=256gb

#PBS -l walltime=01:05:30

#PBS -q short_cpuQ

export OMP_NUM_THREADS=2
/home/damiano.zaccaron/ParallelCNNLayer/src/openmp 51 14