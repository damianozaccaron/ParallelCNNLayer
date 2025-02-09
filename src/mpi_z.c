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
    // Needs to be corrected since it doesn't work without inputs
    int ker_size, max_size;
    if (argv[1]) { 
        ker_size = atoi(argv[1]); 
    } else { 
        ker_size = 3; 
    }
    if (argv[2]) { 
        max_size = atoi(argv[2]); 
    } else { 
        max_size = 10; 
    }
    
    int stride = 1;
    double bias = ((double)rand() / RAND_MAX) - 0.5;
    
    int comm_sz, my_rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0) {
        printf("Kernel size:%d, Stride:%d, Bias:%f\tcores:%d\nn\ttime\n", ker_size, stride, bias, comm_sz);
    }

    int n;
    // n is the size of the input (unpadded) matrix.
    for (n = 6; n < max_size; n ++)
    {
        double tmin = -1.0;
        int nloop = 1000 / n;
        if (nloop == 0) { nloop = 1; }
        double t1, t2;
        int j;
        int NCOLS = pow(2, n);
        int NROWS = pow(2, n);

        int pad = (ker_size - 1) / 2;

        // Padded dimensions (same-padding here)
        int padded_NROWS = NROWS + 2 * pad;
        int padded_NCOLS = NCOLS + 2 * pad;
        
        // Output dimensions
        int out_rows = ((padded_NROWS - ker_size) / stride) + 1;
        int out_cols = ((padded_NCOLS - ker_size) / stride) + 1;
        
        // padded_NROWS should be divisible by comm_sz if scatterv is not implemented
        int base = padded_NROWS / comm_sz;
        int remainder = padded_NROWS % comm_sz;
        int rows_per_process = base + (my_rank < remainder ? 1 : 0);
        int local_out_rows = ((rows_per_process - ker_size) / stride) + 1;

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

        for (j = 0; j < nloop; j++)
        {
            // padded matrix
            double *local_matrix = (double *)malloc(rows_per_process * padded_NCOLS * sizeof(double));
            if(local_matrix == NULL){
                printf("Error: Memory allocation failed for local_matrix on process %d\n", my_rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            // local result
            double *local_res = (double *)malloc(local_out_rows * out_cols * sizeof(double));
            if (local_res == NULL) {
                printf("Error: Memory allocation failed for local_res on process %d\n", my_rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            memset(local_res, 0, local_out_rows * out_cols * sizeof(double));

            double *flattened_kernel = (double *)malloc(ker_size * ker_size * sizeof(double));
            if(flattened_kernel == NULL){
                printf("Error: Memory allocation failed for flattened_kernel on process %d\n", my_rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            
            double *flattened_padded_matrix = NULL;  // ONly process 0
            double *result = NULL;  // Final result
            
            int i, row, col, krow, kcol;
            
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
                free_matrix(padded_matrix, padded_NROWS);
                
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
            // Scatter the flattened padded matrix
            /*MPI_Scatter(flattened_padded_matrix, rows_per_process * padded_NCOLS, MPI_DOUBLE,
                        local_matrix, rows_per_process * padded_NCOLS, MPI_DOUBLE, 0, MPI_COMM_WORLD);*/
            
            MPI_Scatterv(flattened_padded_matrix, sendcounts, displs, MPI_DOUBLE,
                         local_matrix, rows_per_process * padded_NCOLS, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            
            // Each process performs convolution on its local padded block.
            // Iterate over local output indices (using stride).
            for (row = 0; row < local_out_rows; row++){
                for (col = 0; col < out_cols; col++){
                    double sum = bias;

                    int start_row = row * stride;
                    int start_col = col * stride;

                    // Looping over the kernel
                    for (krow = 0; krow < ker_size; krow++){
                        for (kcol = 0; kcol < ker_size; kcol++){
                            int mat_row = start_row + krow;
                            int mat_col = start_col + kcol;
                            sum += local_matrix[mat_row * padded_NCOLS + mat_col] *
                                   flattened_kernel[krow * ker_size + kcol];
                        }
                    } 
                    local_res[row * out_cols + col] = sum;
                }
            }
            
            MPI_Reduce(local_res, result, out_rows * out_cols, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            
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