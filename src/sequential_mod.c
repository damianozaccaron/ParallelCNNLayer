#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#include "random_matrix.c"

#define NUMBER_OF_TESTS 15

int main()
{   
    
    int n;

    int ker_sizes[3] = {3, 25, 51};   
    int bench_idk;
        
    for (bench_idk = 0; bench_idk < 3; bench_idk++)
    {
        int ker_size = ker_sizes[bench_idk];
        printf("Kernel size: %d\nn\ttime (micro)\tseconds\n", ker_size);

        for (n = 14; n < NUMBER_OF_TESTS; n++)
        {
            int j, nloop;
            int NCOLS = pow(2, n);
            int NROWS = pow(2, n);
            double tmin = -1.0;

            nloop = 1000/NCOLS;
            if (nloop == 0) {nloop = 1;}

            for (j = 0; j < nloop; j++)
                {
                double **my_matrix = random_matrix(NROWS, NCOLS);
                double **kernel = random_matrix(ker_size, ker_size);

                int i, j, k, l;
                struct timeval init, end;

                double bias = ((double)rand() / RAND_MAX) - 0.5;
                
                // padding
                int pad = (ker_size - 1) / 2;
                int padded_NROWS = NROWS + 2 * pad;
                int padded_NCOLS = NCOLS + 2 * pad;

                    
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
                            padded_matrix[i + pad][j2 + pad] = my_matrix[i][j2];
                        }
                    }
                
                // Output dimensions
                int result_rows = (padded_NROWS - ker_size) + 1;
                int result_cols = (padded_NCOLS - ker_size) + 1;

                    double **result = (double **)malloc(result_rows * sizeof(double *));
                    for (i = 0; i < result_rows; i++) {
                        result[i] = (double *)malloc(result_cols * sizeof(double));
                        // Initialize the padded matrix rows to zero
                        memset(result[i], 0, result_cols * sizeof(double));
                    }

                gettimeofday(&init, NULL);

                for (i = 0; i < result_rows; i++) {
                    for (j = 0; j < result_cols; j++) {
                        double sum = bias;
                        for (k = 0; k < ker_size; k++) {
                            for (l = 0; l < ker_size; l++) {
                                int row_idx = i + k;
                                int col_idx = j + l;
                                sum += padded_matrix[row_idx][col_idx] * kernel[k][l];
                            }
                        }
                        result[i][j] = relu(sum);
                    }
                }

                gettimeofday(&end, NULL);


                if (tmin < 0 || (end.tv_sec-init.tv_sec) + (end.tv_usec-init.tv_usec)/1000000.0 < tmin) {tmin = (end.tv_sec-init.tv_sec) + (end.tv_usec-init.tv_usec)/1000000.0;}
                


                free_matrix(my_matrix, NROWS);
                free_matrix(kernel, ker_size);
                free_matrix(padded_matrix, padded_NROWS);
                free_matrix(result, result_rows);
                }
            printf("%d\t%.2f\n", n, tmin); 
        }
    }
    return 0;
}
