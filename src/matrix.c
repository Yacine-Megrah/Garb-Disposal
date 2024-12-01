#include "./h/matrix.h"

void free_matrix(int** T, size_t size){
    for(int j=size ; j > -1 ; j--){
        if(T[j])free(T[j]);
    }
    free(T);
}

int **new_matrix(size_t size){
    int **T = NULL;
    T = (int**)malloc(10 * sizeof(int*));
    if(!T){
        return NULL;
    }
    for(int i=0 ; i < 10 ; i++){
        T[i] = NULL;
        T[i] = (int*)malloc(10 * sizeof(int));
        if(!T[i]){
            free_matrix(T, i-1);
            return NULL;
        }
    }
    return T;
}