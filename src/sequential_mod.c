#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>

#include "random_matrix.c"

#define KER_SIZE 3
#define NUMBER_OF_TESTS 10
#define MAX_SIZE 100000

#define STRIDE 1
#define PAD 1

int main()
{   
    printf("Kernel size: %d\nn\ttime (micro)\tseconds\n", KER_SIZE);
    int n;
    int start;
    for (n = start; n < start + NUMBER_OF_TESTS; n++)
    {
        int NCOLS = pow(2, n);
        int NROWS = pow(2, n);
        
        double **my_matrix = random_matrix(NROWS, NCOLS);
        double **kernel = random_matrix(KER_SIZE, KER_SIZE);

        int i, j, k, l, x, y;
        struct timeval init, end;

        double bias = ((double)rand() / RAND_MAX) - 0.5;
        
        // Padding
        int padded_rows = NROWS + 2 * PAD;
        int padded_cols = NCOLS + 2 * PAD;

        double *padded_data = calloc(padded_rows * padded_cols, sizeof(double));
        double **padded_input = malloc(padded_rows * sizeof(double *));
        if (!padded_input) {
            fprintf(stderr, "Allocation failed for padded input.\n");
            exit(1);
        }
        for (i = 0; i < padded_rows; i++) {
            padded_input[i] = &padded_data[i * padded_cols];
            if (!padded_input[i]) {
                fprintf(stderr, "Allocation failed for padded input row %d.\n", i);
                free(padded_data);
                exit(1);
            }
        }

        // Copying the original input into the center of the padded matrix
        for (i = 0; i < NROWS; i++) {
            for (j = 0; j < NCOLS; j++) {
                padded_input[i + PAD][j + PAD] = my_matrix[i][j];
            }
        }
        
        // Output dimensions
        int result_rows = ((padded_rows - KER_SIZE) / STRIDE) + 1;
        int result_cols = ((padded_cols - KER_SIZE) / STRIDE) + 1;

        double *result_data = calloc(result_rows * result_cols, sizeof(double));
        if (!result_data) {
            fprintf(stderr, "Memory allocation failed for result data block\n");
            exit(1);
        }
        double **result = (double **)malloc(result_rows * sizeof(double *));
        if (!result) {
            fprintf(stderr, "Memory allocation failed for result row pointers\n");
            free(result_data);
            exit(1);
        }
        // Filling with zeros
        for (i = 0; i < result_rows; i++) {
            result[i] = &result_data[i * result_cols]; 
        }

        gettimeofday(&init, NULL);

        for (i = 0; i < result_rows; i++) {
            for (j = 0; j < result_cols; j++) {
                double sum = bias;
                for (k = 0; k < KER_SIZE; k++) {
                    for (l = 0; l < KER_SIZE; l++) {
                        int row_idx = i * STRIDE + k;
                        int col_idx = j * STRIDE + l;
                        sum += padded_input[row_idx][col_idx] * kernel[k][l];
                    }
                }
                result[i][j] = relu(sum);
            }
        }

        gettimeofday(&end, NULL);
        long seconds = end.tv_sec - init.tv_sec;
        long micros = ((seconds * 1000000L) + end.tv_usec) - init.tv_usec;
        printf("%d\t%ld\t%ld\n", NROWS, micros,seconds);

        free_matrix(my_matrix, NROWS);
        free_matrix(kernel, KER_SIZE);
        free_matrix(padded_input, padded_rows);
        free_matrix(result, result_rows);
    }
    return 0;
}
