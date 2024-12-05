#!/bin/bash

#PBS -l select=1:ncpus=1:mem=8gb

#PBS -l walltime=0:0:50

#PBS -q short_cpuQ

module load mpich-3.2
mpirun.actual -n 1 /home/raffaele.sinani/ParallelCNNLayer/src/seq