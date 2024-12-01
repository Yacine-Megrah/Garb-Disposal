#ifndef MATRIX_H
#define MATRIX_H

#include <stdlib.h>

/**
 * Creates a new matrix of size m*m
 * 
 * @return A dynamically allocated 2D integer array (matrix)
 *         Returns NULL if memory allocation fails
 */
int **new_matrix(size_t size);

/**
 * Frees the memory allocated for a matrix
 * 
 * @param T The matrix to be freed
 * @param size The number of rows to free (should be the last index used)
 */
void free_matrix(int** T, size_t size);

#endif /* MATRIX_H */