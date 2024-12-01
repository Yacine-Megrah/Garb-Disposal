#include <stdlib.h>
#include <stdio.h>
#include <time.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>

#include "./h/matrix.h"

#define MIN 70
#define MAX 12
#define C 2

typedef enum {
    VIDE = 1,
    PLEINE,
    EN_DECHARGE,
} etat_poubelle;

typedef enum {
    EN_MISSION = 1,
    EN_REPOS,
    EN_RAVETAILLEMENT,
    PRET_POUR_PROG,
} etat_camion;

typedef struct msgbuf{
    long mtype; //id camion
    int code_mission;
    int id_poubelle1, id_poubelle2;
} msgbuf_t;

typedef struct {
    int id;
    int etat;
    int carb;
} camion_t;

void destroy_structures(
    int *d_decharge, int** d_poubelles,
    camion_t* e_camions,
    int tamp_id, int file_msg_id){
    
    if(file_msg_id){
        msgctl(file_msg_id, IPC_RMID, NULL);
        printf("file de msg cloturee\n");
    }
    if(e_camions){
        shmdt(e_camions);
        shmctl(tamp_id, IPC_RMID, NULL);
        printf("tampon cloture\n");
    }
    if(d_poubelles){
        free(d_poubelles);
        printf("d_poubelles cloturee\n");}
    if(d_decharge){
        free(d_decharge);
        printf("d_decharge cloturee\n");}
}

int init_structures(int *d_decharge,
                    int** d_poubelles,
                    camion_t* e_camions,
                    int m, int n,
                    int *tamp_id, int* file_msg_id)
{
    d_decharge = (int*)malloc(m * sizeof(int));
    if (!d_decharge){
        printf("echec malloc dist_decharge\n");
        return 1;
    }

    d_poubelles = new_matrix(m);
    if(!d_poubelles){
        printf("echec malloc dist_decharge\n");
        return 1;
    }

    key_t key_tamp = ftok("./src/main.c", 64);
    *tamp_id = shmget(key_tamp, sizeof(camion_t) * n, 0666 | IPC_CREAT);
    if (*tamp_id == -1){
        printf("echec creation du tampon etats_camions, error[%s]\n", strerror(errno));
        return 1;
    }
    e_camions = (camion_t*)shmat(*tamp_id, NULL, 0);
    
    key_t key_msg = ftok("./src/main.c", 64);
    *file_msg_id = msgget(key_msg, 0666 | IPC_CREAT);
    if(*file_msg_id==-1){
        printf("echec creation de la file de messages Faffec, error[%s]\n", strerror(errno));
        return 1;
    }

    FILE *f_d_poubelles = fopen("d_poubelles.dat", "w+"); 
    FILE *f_d_decharge = fopen("d_poubelles.dat", "w+"); 

    for(int i = 0 ; i < m ; i++){
        for(int j = 0; j < i ; j++ ){
            d_poubelles[i][j] = 1 + (rand()%21);
            d_poubelles[j][i] = d_poubelles[i][j];
            fprintf(f_d_poubelles, "(P[%d,%d]: %d), ", i, j, d_poubelles[i][j]);
        }
        d_poubelles[i][i] = 0;
        d_decharge[i] = 1 + (rand()%21);
        fprintf(f_d_poubelles, "(D[%d]: %d)\n", i, d_decharge[i]);
    }
    fclose(f_d_decharge);
    fclose(f_d_poubelles);
}

void Camion(int id){
    exit(200 + id);
}
void Controlleur(){
    
    exit(200);
}

int main(int argc, char* argv[]){
    int tampid, file_msg_id;
    int m = 10;
    int n = 5;
    int cp = 300;
    int *dist_decharge = NULL;
    int **dist_poubelles = NULL;
    camion_t *etats_camions = NULL;

    srand(time(NULL));

    if(init_structures(dist_decharge, dist_poubelles, etats_camions, m, n, &tampid, &file_msg_id)){
        printf("Erreur: echec init structures\n");
        destroy_structures(dist_decharge, dist_poubelles, etats_camions, tampid, file_msg_id);
        return 1;
    }

    for(int i = 0; i++ ; i <= n){
        if(!i){
            if(fork()==0){
                Controlleur();
            }
        }
        if(fork()==0){
            Camion(i);
        }
    }

    for(int i = 0; i++ ; i <= n){
        wait(NULL);
    }
    
    destroy_structures(dist_decharge, dist_poubelles, etats_camions, tampid, file_msg_id);

    return EXIT_SUCCESS;
}