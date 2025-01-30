#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

double ** random_matrix (int rows, int columns)
{   
    int i, j;
    double ** arr = (double **)malloc(rows * sizeof(double *));

    for (i = 0; i < rows; i++)

    {
        arr[i] = (double *)malloc(columns * sizeof(double));
        for (j = 0; j < columns; j++)
        {
            arr[i][j] = rand() % 10;
        }
    }
    return arr;
}

double * flatten_matrix (double ** matrix,int rows,int columns)
{
    double * result = (double *)malloc(rows*sizeof(double));
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++) {
            result[i * columns + j] = matrix[i][j];
        }
    }
    return result;

}

void free_matrix (double ** matrix, int rows)
{
    int i;
    for (i = 0; i < rows; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}

void print_matrix(double ** matrix, int rows, int columns) {
    int x, y;

    for(x = 0 ; x < rows ; x++) {
        printf(" (");
        for(y = 0 ; y < columns ; y++){
            printf("%f     ", matrix[x][y]);
        }
        printf(")\n");
    }
}

void print_vector(double * vector, int size)
{   
    int i;
    printf("(");
    for (i = 0; i < size; i++)
        {
            printf("%f\t", vector[i]);
        }
    printf(")\n");
}