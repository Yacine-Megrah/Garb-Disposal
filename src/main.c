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

#define R 3
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

typedef struct camion_t{
    int id;
    int etat;
    int carb;
} camion_t;

typedef struct tamp_t{
    int q,cpt;
    camion_t camions[R];
} tamp_t;

void destroy_structures(
    int **d_poubelles,
    tamp_t* e_camions,
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
}

int init_structures(int ***d_poubelles,
                    tamp_t **e_camions,
                    int m, int n,
                    int *tamp_id, int *file_msg_id)
{
    *d_poubelles = new_matrix(m);
    if(*d_poubelles == NULL){
        printf("echec malloc dist_decharge\n");
        return 1;
    }

    key_t key_tamp = ftok("./src/main.c", 64);
    int id = shmget(key_tamp, sizeof(tamp_t) * n, 0666 | IPC_CREAT);
    if (id == -1){
        printf("echec creation du tampon etats_camions, error[%s]\n", strerror(errno));
        return 1;
    }
    *tamp_id = id;
    *e_camions = (tamp_t*)shmat(*tamp_id, NULL, 0);
    
    key_t key_msg = ftok("./src/main.c", 64);
    id = msgget(key_msg, 0666 | IPC_CREAT);
    if(id==-1){
        printf("echec creation de la file de messages Faffec, error[%s]\n", strerror(errno));
        return 1;
    }
    *file_msg_id = id;
    
    printf("[%d ,%d ,%d ,%d]\n", d_poubelles, e_camions, *tamp_id, *file_msg_id);
    
    FILE *f_poubelles = fopen("./bin/dist.dat", "w+");
    for(int i = 0; i < m ; i++){
        (*d_poubelles)[i][i] = 2 + (rand()%9);
        fprintf(f_poubelles, "D[%d,%d]: %d, ", i, i, (*d_poubelles)[i][i]);
        for(int j = i + 1 ; j < m ; j++){
            (*d_poubelles)[i][j] = 1 + (rand()%10);
            (*d_poubelles)[j][i] = (*d_poubelles)[i][j];
            fprintf(f_poubelles, "P[%d,%d]: %d, ", i, j, (*d_poubelles)[i][j]);
        }
    }
    fclose(f_poubelles);

    return 0;
}

void camion_destroy(tamp_t *e_camions){
    if(e_camions){
        printf("\tCamion detach tamp.\n");
        shmdt(e_camions);
    }
}

int camion_init(int id, int tamp_id, int msg_id, tamp_t **e_camions){
    e_camions = shmat(tamp_id, e_camions, 0666);
    if(!e_camions){
        printf("Camion(%d)::echec attachement tampon. err[%s]\n", id, strerror(errno));
        return 1;
    }
    if(msgget(ftok("./src/main.c", 64), 0666) != msg_id){
        printf("\tCamion(%d):: File de messages introuvable\n", id);
        return 1;
    }
    return 0;
}

void Camion(int id, int tamp_id, int msg_id){
    tamp_t *etats_camions = NULL;
    msgbuf_t message;

    if(camion_init(id, tamp_id, msg_id, &etats_camions)){
        printf("Erreur sur Init() Camion[%d]\n", id);
        camion_destroy(etats_camions);
        exit(500 + id);
    }

    int fin = 0;
    while(!fin){

    }

    camion_destroy(etats_camions);
    exit(200 + id);
}
void Controlleur(){
    
    exit(200);
}

int main(int argc, char* argv[]){
    int tampid;
    int file_msg_id;
    int m = 10;
    int n = 5;
    int cp = 300;
    int **dist_poubelles = NULL;
    tamp_t *etats_camions = NULL;

    srand(time(NULL));

    if(init_structures(&dist_poubelles, &etats_camions, m, n, &tampid, &file_msg_id) == 1){
        printf("Erreur: echec init structures\n");
        destroy_structures(dist_poubelles, etats_camions, tampid, file_msg_id);
        return 1;
    }
    printf("before fork()\n");
    int _waits = 0;
    int id = fork();
    if(!id){
        _waits++;
        Controlleur();
    }else if (id == -1){
        printf("fork() failed\n");
    }
    
    for(int i = 0; i++ ; i < n){
        if(!id){
            _waits++;
            Camion(i, tampid, file_msg_id);
        }else if (id == -1){
            printf("fork() failed\n");
        }
    }

    while(_waits){
        wait(NULL);
        _waits--;
    }
    
    printf("before destroy()\n");
    printf("[%d ,%d ,%d ,%d ,%d]\n", dist_poubelles, etats_camions, tampid, file_msg_id);
    destroy_structures(dist_poubelles, etats_camions, tampid, file_msg_id);

    return EXIT_SUCCESS;
}