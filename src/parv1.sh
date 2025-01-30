#!/bin/bash

#PBS -l select=2:ncpus=1:mem=4gb

#PBS -l walltime=0:00:30

#PBS -q short_cpuQ

module load mpich-3.2
mpirun.actual -n 2 /home/raffaele.sinani/ParallelCNNLayer/src/parv1