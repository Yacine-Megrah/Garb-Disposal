#include <stdlib.h>
#include <stdio.h>
#include "./h/matrix.h"

int main(int argc, char *argv[]){
    int **T = NULL;
    T = new_matrix(10);
    T[0][8] = 12;
    printf("%d\n", T[0][8]);
    free_matrix(T, 9);
    return EXIT_SUCCESS;
}