#ifndef RANDOM_MATRIX_H
#define RANDOM_MATRIX_H

#include <stdlib.h> 
#include <stdio.h> 

// Matrix allocation/free
double **random_matrix(int rows, int columns);
void free_matrix(double **matrix, int rows);

// Flattening utilities
double *flatten_matrix(double **matrix, int rows, int columns);
void flatten_matrix_ptr(double *pointer, double **matrix, int rows, int columns);
void random_flat_matrix_ptr(double *pointer, int rows, int columns);

// Debugging utilities
void print_matrix(double **matrix, int rows, int columns);
void print_vector(double *vector, int size);

// Activation function
double relu(double x);

#endif // RANDOM_MATRIX_H