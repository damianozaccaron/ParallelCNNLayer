#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#include "matrix_utilities.c"

int main(int argc, char *argv[])
{
    // Needs to be corrected since it doesn't work without inputs
    int ker_size, max_size;
    if (argv[1] == 0) { 
        ker_size = atoi(argv[1]); 
    } else { 
        ker_size = 3; 
    }
    if (argv[2] == 0) { 
        max_size = atoi(argv[2]); 
    } else { 
        max_size = 13; 
    }

    double bias = ((double)rand() / RAND_MAX) - 0.5;

    int comm_sz, my_rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


    if (my_rank == 0) {
        printf("Kernel size:%d, Bias:%f\tcores:%d\nn\ttime\n", ker_size, bias, comm_sz);
    }
    int n;
    // n is the size of the input (unpadded) matrix.
    for (n = 6; n <= max_size; n++)
    {
        double tmin = -1.0;

        double t1, t2;
        int j;
        int NCOLS = pow(2, n);
        int NROWS = pow(2, n);
        int nloop = 1000 / NCOLS; //This is used later for the number of repetitions of the process
        if (nloop == 0) { nloop = 1; }

        int pad = (ker_size - 1) / 2;

        // Padded dimensions (same-padding here)
        int padded_NROWS = NROWS + 2 * pad;
        int padded_NCOLS = NCOLS + 2 * pad;
        
        // Output dimensions
        int out_rows = (padded_NROWS - (ker_size - 1));
        int out_cols = (padded_NCOLS -  (ker_size - 1));
        
        // padded_NROWS should be divisible by comm_sz if scatterv is not implemented
        int base = padded_NROWS / comm_sz;
        int remainder = padded_NROWS % comm_sz;
        int rows_per_process = base + (my_rank < remainder ? 1 : 0);

        // Initializing vectors of counts and displacements for scatterv
        int *sendcounts = NULL, *displs = NULL;
        if (my_rank == 0) {
            sendcounts = (int *)malloc(comm_sz * sizeof(int));
            displs = (int *)malloc(comm_sz * sizeof(int));
            int offset = 0;
            int p;
            for (p = 0; p < comm_sz; p++) {
                int p_local_rows = base + (p < remainder ? 1 : 0);
                sendcounts[p] = p_local_rows * padded_NCOLS; 
                displs[p] = offset;
                offset += sendcounts[p];
            }
        }

        for (j = 0; j < nloop; j++) //This loop allows for more stable timings by repeating the process many times for low n
        {
            // padded matrix
            double *local_matrix = (double *)malloc(rows_per_process * padded_NCOLS * sizeof(double));
            if(local_matrix == NULL){
                printf("Error: Memory allocation failed for local_matrix on process %d\n", my_rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            // local result
            double *local_res = (double *)malloc(out_rows * out_cols * sizeof(double));
            if (local_res == NULL) {
                printf("Error: Memory allocation failed for local_res on process %d\n", my_rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            memset(local_res, 0, out_rows * out_cols * sizeof(double));

            double *flattened_kernel = (double *)malloc(ker_size * ker_size * sizeof(double));
            if(flattened_kernel == NULL){
                printf("Error: Memory allocation failed for flattened_kernel on process %d\n", my_rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            
            double *flattened_padded_matrix = NULL;  //They must be initialized on every process, but are going to be allocated only on rank 0
            double *result = NULL;  // Final result
            int row_idx, col_idx; 
            int i, k;
            
            if (my_rank == 0)
            {
                double **orig_matrix = random_matrix(NROWS, NCOLS);
                
                double **padded_matrix = (double **)malloc(padded_NROWS * sizeof(double *));
                for (i = 0; i < padded_NROWS; i++) {
                    padded_matrix[i] = (double *)malloc(padded_NCOLS * sizeof(double));
                    // Initialize the padded matrix rows to zero
                    memset(padded_matrix[i], 0, padded_NCOLS * sizeof(double));
                }
                // Copying the original matrix into the center of the padded matrix
                int j2;
                for (i = 0; i < NROWS; i++) {
                    for (j2 = 0; j2 < NCOLS; j2++) {
                        padded_matrix[i + pad][j2 + pad] = orig_matrix[i][j2];
                    }
                }

                flattened_padded_matrix = flatten_matrix(padded_matrix, padded_NROWS, padded_NCOLS);
                
                // Padding done, now kernel
                double ** kernel = random_matrix(ker_size, ker_size); 
                flatten_matrix_ptr(flattened_kernel, kernel, ker_size, ker_size);
                
                free_matrix(kernel, ker_size);
                free_matrix(orig_matrix, NROWS);
                free_matrix(padded_matrix, padded_NROWS);  //We don't need these matrices anymore
                
                // Allocate the final result
                result = (double *)malloc(out_rows * out_cols * sizeof(double));
                if(result == NULL){
                    printf("Error: Memory allocation failed for result on process %d\n", my_rank);
                    MPI_Abort(MPI_COMM_WORLD, 1);
                }
            }
            
            MPI_Barrier(MPI_COMM_WORLD);
            t1 = MPI_Wtime();
            // Broadcast the kernel
            MPI_Bcast(flattened_kernel, ker_size * ker_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

            // Scatter the padded matrix            
            MPI_Scatterv(flattened_padded_matrix, sendcounts, displs, MPI_DOUBLE,
                         local_matrix, rows_per_process * padded_NCOLS, MPI_DOUBLE, 0, MPI_COMM_WORLD);


            for (i = 0; i < rows_per_process * NCOLS; i++)
            {
                for (k = 0; k < ker_size * ker_size; k++)
                {
                    row_idx = i / NCOLS + (rows_per_process * my_rank) - k / ker_size; //This is the row of the result matrix our product would be added to (if the result were a matrix)
                    col_idx = i % NCOLS - k % ker_size; //Same for the columns

                    if (row_idx >= 0 && row_idx < NROWS - (ker_size - 1) &&
                        col_idx >= 0 && col_idx < NCOLS - (ker_size - 1))  //This condition is to avoid writing outside the result matrix
                            {
                                local_res[row_idx *(NCOLS - (ker_size - 1)) + col_idx] += local_matrix[i] * flattened_kernel[k];
                            }
                }

            } 




            MPI_Reduce(local_res, result, out_rows * out_cols, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

            if (my_rank == 0)
            {   
                int l;
                for (l = 0; l < out_cols*out_rows; l++)
                {
                    result[l] = relu(result[l] + bias);
                }
            } //We sum the bias and apply the activation function to the result matrix.

            t2 = MPI_Wtime() - t1;
            if (tmin < 0 || t2 < tmin) { tmin = t2; }
            
            if (my_rank == 0) {
                free(result);
                free(flattened_padded_matrix);
            }
            free(local_res);
            free(local_matrix);
            free(flattened_kernel); 
        } 
        
        if (my_rank == 0) { 
            printf("%d\t%f\n", n, tmin); 
        } 
    }
    
    MPI_Finalize();
    return 0;
}