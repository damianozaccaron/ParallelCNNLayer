#!/bin/bash

#PBS -l select=1:ncpus=1:mem=64gb

#PBS -l walltime=03:00:00

#PBS -q short_cpuQ

module load mpich-3.2
mpirun.actual -n 1 /home/damiano.zaccaron/ParallelCNNLayer/src/seq 3 14