#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>



#include "random_matrix.c"

/*
#define NROWS 5
#define NCOLS 5
*/
#define KER_SIZE 3
#define MAX_SIZE 50000

int main()
{   
    int comm_sz, my_rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


    printf("Kernel size:%d\nn\ttime\n", KER_SIZE);
    int n;
    for (n = 32768; n < MAX_SIZE; n*=2)
        {
        int NCOLS = n;
        int NROWS = n;
        int rows_per_process = NROWS / comm_sz;
        double * local_matrix  = (double *)malloc(rows_per_process * NCOLS * sizeof(double));
        double * local_res =(double *)malloc((NROWS - (KER_SIZE - 1)) * (NCOLS - (KER_SIZE - 1)) * sizeof(double));
        double * result;
        double * flattened_kernel = (double *) malloc (KER_SIZE * KER_SIZE * sizeof(double));
        double * flattened_matrix;
        int i, k, col_idx, row_idx;

        if (my_rank == 0)
        {
            
            double ** my_matrix = random_matrix(NROWS, NCOLS); // initialize a random matrix
            double ** kernel = random_matrix(KER_SIZE, KER_SIZE); //initialize a random kernel
            flattened_matrix = (double *) malloc (NROWS * NCOLS * sizeof(double));
            print_matrix(my_matrix, NROWS, NCOLS);
            printf("\n");
            print_matrix(kernel, KER_SIZE, KER_SIZE);
            printf("\n");

            flatten_matrix_ptr(flattened_kernel, kernel, KER_SIZE, KER_SIZE);
            flatten_matrix_ptr(flattened_matrix, my_matrix, NROWS, NCOLS);

            result =(double *)malloc((NROWS - (KER_SIZE - 1)) * (NCOLS - (KER_SIZE - 1)) * sizeof(double));
            
            free_matrix(my_matrix, NROWS);  //WE should be able to free the allocated space for the original matrix and kernal
            free_matrix(kernel, KER_SIZE);

        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast (flattened_kernel, KER_SIZE * KER_SIZE, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        /*
        printf("I am node %d\n", my_rank);
        
        print_vector(flattened_kernel, KER_SIZE*KER_SIZE);
*/
        MPI_Scatter(flattened_matrix, rows_per_process * NCOLS, MPI_DOUBLE, local_matrix, rows_per_process * NCOLS, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        /*printf("I am node %d\n", my_rank);
        print_vector(local_matrix, rows_per_process * NCOLS);
        */
        free(flattened_kernel);
        free(flattened_matrix); //If we freed later we would need to allocate double the size of the memory, probably slower we can see later
          

        for (i = 0; i < rows_per_process * NCOLS; i++)
        {
            for (k = 0; k < KER_SIZE * KER_SIZE; k++)
            {
                row_idx = i / NCOLS + (rows_per_process * my_rank) - k / KER_SIZE; //This is the row of the result matrix our product would be added to (if the result were a matrix)
                col_idx = i % NCOLS - k % KER_SIZE; //Same for the columns
                // printf("Row index: %d, Col index: %d\n", row_idx, col_idx);               
                if (row_idx >= 0 && row_idx < NROWS - (KER_SIZE - 1) &&
                    col_idx >= 0 && col_idx < NCOLS - (KER_SIZE - 1)) 
                        {
                            local_res[row_idx * rows_per_process + col_idx] += local_matrix[i] * flattened_kernel[k];
                            if(my_rank == 31) {printf("aggiungo nel posto %d il numero %f\n", row_idx * rows_per_process + col_idx, local_matrix[i] * flattened_kernel[k]);}
                        }
            }

        }
        
        MPI_Reduce(local_res, result, (NROWS - (KER_SIZE - 1)) * (NCOLS - (KER_SIZE - 1)), MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);
        if (my_rank == 0)
        {
        print_vector(result, (NROWS - (KER_SIZE - 1)) * (NCOLS - (KER_SIZE - 1)));
        }

        free(local_res);
        free(local_matrix);

        if (my_rank == 0)
        {
            free(result);
        }

        }

    MPI_Finalize();
    return 0;
}

