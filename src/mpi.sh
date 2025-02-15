#!/bin/bash

#PBS -l select=64:ncpus=1:mem=64gb

#PBS -l walltime=00:05:00

#PBS -q short_cpuQ

module load mpich-3.2
mpirun.actual -n 64 /home/damiano.zaccaron/ParallelCNNLayer/src/mpi 3 10