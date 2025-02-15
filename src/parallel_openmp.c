#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>
#include <omp.h>
#include <string.h>

#include "matrix_utilities.c"

int main(int argc, char *argv[]) {   
    // Needs to be corrected since it doesn't work without inputs
    int KER_SIZE, max_size;
    if (argv[1]) { 
        KER_SIZE = atoi(argv[1]); 
    } else { 
        KER_SIZE = 3; 
    }
    if (argv[2]) { 
        max_size = atoi(argv[2]); 
    } else { 
        max_size = 14; 
    }
    int pad = (KER_SIZE - 1) / 2;
    
    // Introducing a bias randomly chosen between -0.5 and 0.5 
    // in reality it would be optimised through backpropagation
    double bias = ((double)rand() / RAND_MAX) - 0.5;

    int max_threads = omp_get_max_threads();
    printf("Kernel size:%d, Bias:%f, Threads:%d\nn\ttime\n", KER_SIZE, bias, max_threads);
    
    int n;
    // n is the size of the input (unpadded) matrix.
    for (n = 14; n <= max_size; n ++){
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

        // Implementing padding matrix
        int padded_rows = NROWS + 2 * pad;
        int padded_cols = NCOLS + 2 * pad;

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
                padded_input[i + pad][j + pad] = input_matrix[i][j];
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

        // Repeat the process many times and take the minimum (?)
        int nloop = 1000 / NCOLS;
        if (nloop == 0) {nloop = 1;}
        double tmin = -1.0;

        int rep;
        for (rep = 0; rep < nloop; rep++) {
            memset(result_data, 0, result_rows * result_cols * sizeof(double));

            double t_start = omp_get_wtime();

            #pragma omp parallel for collapse(2) private(k, l) schedule(static)
            for (x = 0; x < result_rows; x++) {
                for (y = 0; y < result_cols; y++) {
                    double sum = bias; // Start with the bias
                    // Perform convolution over the kernel window
                    for (k = 0; k < KER_SIZE; k++) {
                        for (l = 0; l < KER_SIZE; l++) {
                            int row = x + k;
                            int col = y + l;
                            sum += padded_input[row][col] * kernel[k][l];
                        }
                    }
                    // Apply ReLU
                    result[x][y] = relu(sum);
                }
            }

            double t_end = omp_get_wtime();
            double elapsed = t_end - t_start;
            if (tmin < 0 || elapsed < tmin) {
                tmin = elapsed;
            }
        }

        printf("%d\t%.06f\n", n, tmin);

        free_matrix(input_matrix, NROWS);
        free_matrix(kernel, KER_SIZE);
        
        free(padded_data);  
        free(padded_input);  
        free(result_data);
        free(result);
    }
    return 0;
}