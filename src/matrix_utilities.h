#ifndef MATRIX_UTILITES_H
#define MATRIX_UTILITES_H

#include <stdlib.h> 
#include <stdio.h> 

// Matrix allocation/free
double **random_matrix(int rows, int columns);
void free_matrix(double **matrix, int rows);

// Flattening utilities
double *flatten_matrix(double **matrix, int rows, int columns); //returns a pointer to a flattened matrix
void flatten_matrix_ptr(double *pointer, double **matrix, int rows, int columns); //flattens a matrix into a pointer already allocated
void random_flat_matrix_ptr(double *pointer, int rows, int columns);  //creates a random flattened matrix

// Debugging utilities
void print_matrix(double **matrix, int rows, int columns);
void print_vector(double *vector, int size);

// Activation function
double relu(double x);

#endif 