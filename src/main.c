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

typedef enum E_poubelle {
    VIDE = 1,
    PLEINE,
    EN_DECHARGE,
} E_poubelle;

typedef enum E_camion {
    EN_MISSION = 1,
    EN_REPOS,
    EN_RAVETAILLEMENT,
    PRET_POUR_PROG,
} E_camion;

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

void destroy_ipc(
    tamp_t* e_camions,
    int tamp_id, int file_msg_id){
    
    if(file_msg_id > -1){
        msgctl(file_msg_id, IPC_RMID, NULL);
        printf("file de msg cloturee\n");
    }
    if(e_camions){
        shmdt(e_camions);
        shmctl(tamp_id, IPC_RMID, NULL);
        printf("tampon cloture\n");
    }
}

int init_ipc(tamp_t **e_camions, int *tamp_id, int *file_msg_id){

    key_t key_tamp = ftok("./src/main.c", 64);
    int id = shmget(key_tamp, sizeof(tamp_t), 0666 | IPC_CREAT);
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

    return 0;
}

void Camion(int id, int _tamp_id, int _msg_id){

    int tamp_id, msg_id;
    tamp_t *etats_camions = NULL;
    msgbuf_t message;

    if(init_ipc(&etats_camions, &tamp_id, &msg_id) == 1 || _tamp_id != tamp_id || _msg_id != msg_id){
        printf("Erreur init Camion[%d], err{%s}\n", id, strerror(errno));
        if(etats_camions)shmdt(etats_camions);
        exit(500+id);
    }

    int fin = 0;
    while(!fin){

    }

    if(etats_camions)shmdt(etats_camions);
    exit(200 + id);
}

void Controlleur(int n, int m, int _tamp_id, int _msg_id){
    E_camion e_camions[n];
    E_poubelle e_poubelles[m];
    
    int tamp_id, msg_id;
    tamp_t *etats_camions = NULL;
    msgbuf_t message;

    if(init_ipc(&etats_camions, &tamp_id, &msg_id) == 1 || _tamp_id != tamp_id || _msg_id != msg_id){
        printf("Erreur init Controlleur, err{%s}\n", strerror(errno));
        if(etats_camions)shmdt(etats_camions);
        exit(500);
    }

    if(etats_camions)shmdt(etats_camions);
    exit(200);
}

int main(int argc, char* argv[]){
    int tampid;
    int file_msg_id;
    int m = 10;
    int n = 5;
    int cp = 300;
    tamp_t *etats_camions = NULL;

    srand(time(NULL));

    if(init_ipc(&etats_camions, &tampid, &file_msg_id) == 1){
        printf("Erreur: echec init structures\n");
        destroy_ipc(etats_camions, tampid, file_msg_id);
        return 1;
    }
    
    int _waits = 0;
    int id = fork();
    if(!id){
        _waits++;
        Controlleur(n, m, tampid, file_msg_id);
    }else if (id == -1){
        printf("fork() failed\n");
    }
    
    for(int i = 1; i++ ; i <= n){
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
    
    destroy_ipc(etats_camions, tampid, file_msg_id);

    return EXIT_SUCCESS;
}