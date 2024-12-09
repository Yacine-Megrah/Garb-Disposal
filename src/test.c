#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include "./h/matrix.h"

void* create_matrix(int **T){
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
    return (void*)T;
}

int main(int argc, char *argv[]){
    int **T = NULL;
    // T = (int**)create_matrix(T);
    T = new_matrix(10);
    T[0][8] = 12;
    printf("%d\n", T[0][8]);
    free_matrix(T, 9);

    int id = msgget(ftok("./src/test.c", 102), 0666 | IPC_CREAT);
    printf("id msgQ: %d\n", id);

    enum myEnum {A=12 , B} enu[18] = {[0 ... 17] = 1};
    for(int i = 0 ; i < 18 ; i++){
        printf("enum[%d] = %d, ", i, enu[i]);
    }printf("\n");

    return EXIT_SUCCESS;
}