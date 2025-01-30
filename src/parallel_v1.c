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
#define NUMBER_OF_TESTS 3


int main()
{   
    int comm_sz, my_rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    //printf("n\ttime\n");
    int n;
    for (n = 2; n < NUMBER_OF_TESTS; n++)
        {
        int NCOLS = pow(2, n);
        int NROWS = pow(2, n);
        int rows_per_process = NROWS / comm_sz;
        double * local_matrix  = (double *)malloc(rows_per_process * NCOLS * sizeof(double));
        double * local_res =(double *)malloc((NROWS - (KER_SIZE - 1)) * (NCOLS - (KER_SIZE - 1)) * sizeof(double));
        double * flattened_kernel = (double *)malloc(KER_SIZE * KER_SIZE * sizeof(double));
        double * flattened_matrix  = (double *)malloc(NROWS * NCOLS * sizeof(double));
        double * result;
        if (my_rank == 0)
        {
            double ** my_matrix = random_matrix(NROWS, NCOLS); // initialize a random matrix
            double ** kernel = random_matrix(KER_SIZE, KER_SIZE); //initialize a random kernel
            print_matrix(my_matrix, NROWS, NCOLS);
            printf("\n");
            print_matrix(kernel, KER_SIZE, KER_SIZE);
            printf("\n");
            flattened_matrix = flatten_matrix(my_matrix, NROWS, NCOLS); //we need to flatten it to send it via MPI
            flattened_kernel = flatten_matrix(kernel, KER_SIZE, KER_SIZE);
            free_matrix(my_matrix, NROWS);
            free_matrix(kernel, KER_SIZE);
            print_vector(flattened_kernel, KER_SIZE*KER_SIZE);
        }
      
        MPI_Bcast (flattened_kernel, KER_SIZE * KER_SIZE, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        printf("I am node %d\n", my_rank);
        
        print_vector(flattened_kernel, KER_SIZE*KER_SIZE);

        MPI_Scatter(flattened_matrix, rows_per_process * NCOLS, MPI_DOUBLE, local_matrix, rows_per_process * NCOLS, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        printf("I am node %d\n", my_rank);
        //print_vector(local_matrix, rows_per_process * NCOLS);
        if (my_rank == 0)
        {
            free(flattened_kernel);
            free(flattened_matrix); //If we freed later we would need to allocate double the size of the memory, probably slower we can see later
            result =(double *)malloc((NROWS - (KER_SIZE - 1)) * (NCOLS - (KER_SIZE - 1)) * sizeof(double *));
        }


        /*
        print_matrix(my_matrix, NROWS, NCOLS);
        printf("\n");
        print_matrix(kernel, KER_SIZE, KER_SIZE);
        printf("\n");

        gettimeofday(&init, NULL);

        for (i = 0; i < NROWS; i++) //Iterates through the rows of the original matrix
        {   
            printf("Entering ROW %d\n", i);
            for (j = 0; j < NCOLS; j++) //Iterates through the columns
            {
                printf("Entering COLUMN %d\n", j);

                for (k = 0; k < KER_SIZE; k++) //Iterates through the rows of the kernel
                {
                    for (l = 0; l < KER_SIZE; l++) //Iterates through the columns of the kernel
                    {
                        row_idx = i - k;//This is because the first row of the kernel only adds to the second line (so -(-1) = +1)
                        col_idx = j - l;

                        if (row_idx >= 0 && row_idx < NROWS - (KER_SIZE - 1) &&
                            col_idx >= 0 && col_idx < NCOLS - (KER_SIZE - 1)) 
                            {
                                result[row_idx][col_idx] += my_matrix[i][j] * kernel[k][l];
                            }
                    }
                }
            }
        }

        gettimeofday(&end, NULL);
        
        printf("%d\t%ld", NROWS, init.tv_sec - end.tv_sec);
        */
        int i, k, col_idx, row_idx;
        for (i = 0; i < rows_per_process * NCOLS; i++)
        {
            for (k = 0; k < KER_SIZE * KER_SIZE; k++)
            {
                row_idx = i / rows_per_process - k / KER_SIZE;
                col_idx = i % NCOLS - k % KER_SIZE;
                printf("Row index: %d, Col index: %d\n", row_idx, col_idx);
                if (row_idx >= 0 && row_idx < NROWS - (KER_SIZE - 1) &&
                    col_idx >= 0 && col_idx < NCOLS - (KER_SIZE - 1)) 
                        {
                            local_res[row_idx * rows_per_process + col_idx] += local_matrix[i] * flattened_kernel[k];
                            printf("aggiungo nel posto %d il numero %f\n", row_idx * rows_per_process + col_idx, local_matrix[i] * flattened_kernel[k]);
                        }
            }

        }
        printf("Fine del loop\n");
        /*
        MPI_Reduce(&local_res, &result, (NROWS - (KER_SIZE - 1)) * (NCOLS - (KER_SIZE - 1)), MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        
        printf("%f", result[0]);
        */
        free(local_res);
        free(local_matrix);
        free(flattened_kernel);

        if (my_rank == 0)
        {
            free(result);
        }

        }

    MPI_Finalize();
    return 0;
}

