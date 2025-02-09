#!/bin/bash

#PBS -l select=2:ncpus=10:mem=64gb 

#PBS -l walltime=01:00:30

#PBS -q short_cpuQ

module load mpich-3.2
mpirun.actual -n 20 /home/damiano.zaccaron/ParallelCNNLayer/src/mpi_z 3 10