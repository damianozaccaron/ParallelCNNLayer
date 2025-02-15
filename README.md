# ParallelCNNLayer

Parallel implementation of a convolutional neural network layer on CPU using the MPI and OpenMP paradigms.

Project for the HPC for Data Science Course at UniTrento, by Damiano Zaccaron and Raffaele Sinani

# Results

The tests folder contains our benchmark results that were also used for our report analysis.

# Running the applications

To run the MPI application it must first be compiled:
```
module load mpich-3.2
mpicc -g -Wall -o src/mpi_z src/mpi_z.c -lm
```
Then run the PBS submission:
```
qsub src/mpi.sh
```
changing the input parameters as necessary

The same must be done for the OMP application:
```
gcc -g -Wall -fopenmp -o src/openmp src/parallel_openmp.c -lm
```
Then run the PBS submission:
```
qsub src/omp.sh
```
The functions take as arguments $KER_SIZE and $MAX_SIZE
