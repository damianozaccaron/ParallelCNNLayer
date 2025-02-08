#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>
#include <omp.h>

#include "random_matrix.c"

#define KER_SIZE 3
#define NUMBER_OF_TESTS 10

#define STRIDE 1
#define PAD 1

int main() {   
    printf("n\ttime\tseconds\n");
    int n;

    int start;
    for (n = start; n < start + NUMBER_OF_TESTS; n++) {
        int NCOLS = pow(2, n);
        int NROWS = pow(2, n);

        int i, j, x, y, k, l;
    
        double **input_matrix = random_matrix(NROWS, NCOLS);
        // Checking if the matrix has been loaded correctly
        if (!input_matrix) { 
            fprintf(stderr, "Failed to allocate input matrix.\n");
            exit(1);
        }
        double **kernel = random_matrix(KER_SIZE, KER_SIZE);
        // Same check for the kernel
        if (!kernel) {
            fprintf(stderr, "Failed to allocate kernel matrix.\n");
            exit(1);
        }

        struct timeval init, end;

        // Introducing a bias randomly chosen between -0.5 and 0.5 
        // in reality it would be optimised through backpropagation
        double bias = ((double)rand() / RAND_MAX) - 0.5;

        // Implementing padding matrix
        int padded_rows = NROWS + 2 * PAD;
        int padded_cols = NCOLS + 2 * PAD;

        double *padded_data = calloc(padded_rows * padded_cols, sizeof(double));
        if (!padded_data) {
            fprintf(stderr, "Memory allocation failed for padded data block\n");
            exit(1);
        } 
        double **padded_input = malloc(padded_rows * sizeof(double *));
        if (!padded_input) {
            fprintf(stderr, "Allocation failed for padded input.\n");
            exit(1);
        }
        for (i = 0; i < padded_rows; i++) {
            padded_input[i] = &padded_data[i * padded_cols];
        }

        // Copying the input amtrix into the center of the padded matrix.
        for (i = 0; i < NROWS; i++) {
            for (j = 0; j < NCOLS; j++) {
                padded_input[i + PAD][j + PAD] = input_matrix[i][j];
            }
        }

        // Initializing result matrix (dimensions = floor of (input-kernel+2padding)/stride)+1
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

        #pragma omp parallel for collapse(2) private(k, l) schedule(static)
        for (x = 0; x < result_rows; x++) {
            for (y = 0; y < result_cols; y++) {
                double sum = bias; // we start with the bias

                // Going over the kernel to perform convolution
                for (k = 0; k < KER_SIZE; k++) { // kernel rows
                    for (l = 0; l < KER_SIZE; l++) { // kernel columns
                        int row = x * STRIDE + k;
                        int col = y * STRIDE + l;
                        sum += padded_input[row][col] * kernel[k][l];
                    }
                }
                // Applying the activation function
                result[x][y] = relu(sum);  // No race condition since x and y are unique for each thread
            }
        }

        gettimeofday(&end, NULL);
        
        long seconds = end.tv_sec - init.tv_sec;
        long micros = ((seconds * 1000000) + end.tv_usec) - (init.tv_usec);
        printf("%d\t%ld\t%ld\n", NROWS, micros, seconds);

        free_matrix(input_matrix, NROWS);
        free_matrix(kernel, KER_SIZE);
        free_matrix(result, result_rows);
        free_matrix(padded_input,padded_rows);
    }
    return 0;
}