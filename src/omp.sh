#!/bin/bash

#PBS -l select=1:ncpus=10:mem=16gb

#PBS -l walltime=00:05:30

#PBS -q short_cpuQ

export OMP_NUM_THREADS=10
/home/damiano.zaccaron/ParallelCNNLayer/src/openmp