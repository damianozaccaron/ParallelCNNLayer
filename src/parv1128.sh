#!/bin/bash

#PBS -l select=128:ncpus=1:mem=64gb -l place=scatter:excl

#PBS -l walltime=01:00:30

#PBS -q short_cpuQ

module load mpich-3.2
mpirun.actual -n 128 /home/raffaele.sinani/ParallelCNNLayer/src/parv1 25 17000