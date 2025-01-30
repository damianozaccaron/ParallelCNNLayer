#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>


#include "random_matrix.c"

/*
#define NROWS 5
#define NCOLS 5
*/
#define KER_SIZE 3
#define NUMBER_OF_TESTS 10

int main()
{   
    printf("n\ttime\n");
    int n;
    for (n = 2; n < NUMBER_OF_TESTS; n++)
        {
        int NCOLS = pow(2, n);
        int NROWS = pow(2, n);
    
        double ** my_matrix = random_matrix(NROWS, NCOLS);
        double ** kernel = random_matrix(KER_SIZE, KER_SIZE);
        int i,j, k, l, x, y;
        struct timeval init, end;
        double ** result =(double **)malloc((NROWS - (KER_SIZE - 1)) * sizeof(double *));
        for (x = 0; x < NROWS - (KER_SIZE - 1); x++) {
            result[x] = malloc((NCOLS - (KER_SIZE - 1)) * sizeof(double));
            for (y = 0; y < NCOLS - (KER_SIZE - 1); y++) {
                result[x][y] = 0.0; // Initialize each element to 0
            }
        }
        /*
        print_matrix(my_matrix, NROWS, NCOLS);
        printf("\n");
        print_matrix(kernel, KER_SIZE, KER_SIZE);
        printf("\n");*/
        int row_idx, col_idx; //Not necessary, let's put it for now
        /*
        Not really necessary in the sequential version
        double * flattened_matrix = flatten_matrix(my_matrix, NROWS, NCOLS);
        */


        gettimeofday(&init, NULL);

        for (i = 0; i < NROWS; i++) //Iterates through the rows of the original matrix
        {   
//            printf("Entering ROW %d\n", i);
            for (j = 0; j < NCOLS; j++) //Iterates through the columns
            {
//                printf("Entering COLUMN %d\n", j);

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
        
        printf("%d\t%ld\n", NROWS, end.tv_sec - init.tv_sec );

//        print_matrix(result, NROWS - (KER_SIZE - 1), NCOLS - (KER_SIZE - 1));
        
        free_matrix(my_matrix, NROWS);
        free_matrix(kernel, KER_SIZE);
        free_matrix(result, NROWS - (KER_SIZE - 1));
        }


    return 0;
}

