#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#include "random_matrix.c"

int main(int argc, char *argv[])
{
    int ker_size, max_size;
    if (argv[1] != 0) {ker_size = atoi(argv[1]);}
    else {ker_size = 3;}
    if (argv[2] != 0) {max_size = atoi(argv[2]);}
    else {max_size = 9000;}
    int comm_sz, my_rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0){printf("Kernel size:%d\tn.cores:%d\nn\ttime\n", ker_size, comm_sz);}
    int n;
    for (n = comm_sz; n < max_size; n*=2)
        {
        double tmin = -1.0;
        int nloop;
        nloop = 1000/n;
        if (nloop == 0) {nloop = 1;}
        double t1, t2;
        int j;
        int NCOLS = n;
        int NROWS = n;
        int rows_per_process = NROWS / comm_sz;
        for (j = 0; j < nloop; j++)
            {
            double * local_matrix  = (double *)malloc(rows_per_process * NCOLS * sizeof(double));

            double * local_res = (double *)malloc((NROWS - (ker_size - 1)) * (NCOLS - (ker_size - 1)) * sizeof(double));
            if (local_res == NULL) {
                printf("Error: Memory allocation failed for local_res on process %d\n", my_rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            if (local_matrix == NULL) {
                printf("Error: Memory allocation failed for local_res on process %d\n", my_rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            // Initialize local_res to zero before accumulation
            memset(local_res, 0, (NROWS - (ker_size - 1)) * (NCOLS - (ker_size - 1)) * sizeof(double));

            double * flattened_kernel = (double *) malloc (ker_size * ker_size * sizeof(double));
            double * flattened_matrix;
            double * result = NULL;
            int i, k, col_idx, row_idx;


            if (my_rank == 0)
            {

                flattened_matrix = (double *)malloc(NROWS*NCOLS*sizeof(double));

                double ** kernel = random_matrix(ker_size, ker_size); //initialize a random kernel

                flatten_matrix_ptr(flattened_kernel, kernel, ker_size, ker_size);
                
                random_flat_matrix_ptr(flattened_matrix, NROWS, NCOLS); //Initialize a random flattened matrix

                free_matrix(kernel, ker_size);
                result = (double *)malloc((NROWS - (ker_size - 1)) * (NCOLS - (ker_size - 1)) * sizeof(double));
                
            }


            MPI_Barrier(MPI_COMM_WORLD);

            t1 = MPI_Wtime();
            MPI_Bcast (flattened_kernel, ker_size * ker_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
            MPI_Scatter(flattened_matrix, rows_per_process * NCOLS, MPI_DOUBLE, local_matrix, rows_per_process * NCOLS, MPI_DOUBLE, 0, MPI_COMM_WORLD);      
            

            for (i = 0; i < rows_per_process * NCOLS; i++)
            {
                for (k = 0; k < ker_size * ker_size; k++)
                {
                    row_idx = i / NCOLS + (rows_per_process * my_rank) - k / ker_size; //This is the row of the result matrix our product would be added to (if the result were a matrix)
                    col_idx = i % NCOLS - k % ker_size; //Same for the columns

                    if (row_idx >= 0 && row_idx < NROWS - (ker_size - 1) &&
                        col_idx >= 0 && col_idx < NCOLS - (ker_size - 1)) 
                            {
                                local_res[row_idx *(NCOLS - (ker_size - 1)) + col_idx] += local_matrix[i] * flattened_kernel[k];
                            }
                }

            } 


            MPI_Reduce(local_res, result, (NROWS - (ker_size - 1)) * (NCOLS - (ker_size - 1)), MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            t2 = MPI_Wtime() - t1;
            if (tmin < 0 || t2 < tmin) {tmin = t2;}

            if (my_rank == 0) {
                free(result);
                free(flattened_matrix);}

            free(local_res);
            free(local_matrix);
            free(flattened_kernel);
            }

        if (my_rank == 0) {printf("%d\t%f\n", n, tmin);}
        }
    MPI_Finalize();
    return 0;
}


