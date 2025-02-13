#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>


#include "random_matrix.c"

/*
#define NROWS 5
#define NCOLS 5
*/
#define MAX_SIZE 10000

int main()
{   
    int n;

    int ker_sizes[5] = {3, 11, 25, 51, 101};   
    int bench_idk;
        
    for (bench_idk = 0; bench_idk < 4; bench_idk++)
    {
        int ker_size = ker_sizes[bench_idk];
        printf("Kernel size:%d\nn\ttime\n", ker_size);

        for (n = 16; n < MAX_SIZE; n*=2)
            {
            int j, nloop;
            int NCOLS = n;
            int NROWS = n;
            double tmin = -1.0;
            
            nloop = 1000/n;
            if (nloop == 0) {nloop = 1;}

            for (j = 0; j < nloop; j++)
                {
                struct timeval init, end;
                int i,j, k, l, x, y;
                double ** my_matrix = random_matrix(NROWS, NCOLS);
                double ** kernel = random_matrix(ker_size, ker_size);
                double ** result =(double **)malloc((NROWS - (ker_size - 1)) * sizeof(double *));

                for (x = 0; x < NROWS - (ker_size - 1); x++) {
                    result[x] = malloc((NCOLS - (ker_size - 1)) * sizeof(double));
                    for (y = 0; y < NCOLS - (ker_size - 1); y++) {
                        result[x][y] = 0.0; // Initialize each element to 0
                    }
                }
                /*
                print_matrix(my_matrix, NROWS, NCOLS);
                printf("\n");
                print_matrix(kernel, ker_size, ker_size);
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

                        for (k = 0; k < ker_size; k++) //Iterates through the rows of the kernel
                        {
                            for (l = 0; l < ker_size; l++) //Iterates through the columns of the kernel
                            {
                                row_idx = i - k;//This is because the first row of the kernel only adds to the second line (so -(-1) = +1)
                                col_idx = j - l;

                                if (row_idx >= 0 && row_idx < NROWS - (ker_size - 1) &&
                                    col_idx >= 0 && col_idx < NCOLS - (ker_size - 1)) 
                                    {
                                        result[row_idx][col_idx] += my_matrix[i][j] * kernel[k][l];
                                    }
                            }
                        }
                    }
                }

                gettimeofday(&end, NULL);
                
                if (tmin < 0 || (end.tv_sec-init.tv_sec) + (end.tv_usec-init.tv_usec)/1000000.0 < tmin) {tmin = (end.tv_sec-init.tv_sec) + (end.tv_usec-init.tv_usec)/1000000.0;}

        //        print_matrix(result, NROWS - (ker_size - 1), NCOLS - (ker_size - 1));
                
                free_matrix(my_matrix, NROWS);
                free_matrix(kernel, ker_size);
                free_matrix(result, NROWS - (ker_size - 1));
                }
            printf("%d\t%.2f\n", NROWS, tmin);
            }
        }
    return 0;
}

