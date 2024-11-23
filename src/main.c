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

#define TampKEY ftok("Tfmission", 64)
#define FileMsgKEY ftok("Faffec", 64)

#define MIN 70
#define MAX 12
#define C 2

typedef struct msgbuf{
    long mtype;
    char buff[64];
} msgbuf_t;

typedef struct {
    int id;
    int etat;
    const char *cursus;
} camion_t;

void destroy_structures(
    int *d_decharge, int* d_poubelles,
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
                    int* d_poubelles,
                    camion_t* e_camions,
                    int m, int n,
                    int *tamp_id, int* file_msg_id)
{
    d_decharge = (int*)malloc(m * sizeof(int));
    if (!d_decharge){
        printf("echec malloc dist_decharge\n");
        return 1;
    }
    d_poubelles = (int*)malloc(m * m * sizeof(int));
    if (!d_poubelles){
        printf("echec malloc dist_poubelles\n");
        return 1;
    }

    *tamp_id = shmget((key_t)TampKEY, sizeof(camion_t) * n, 0666 | IPC_CREAT);
    if (!(*tamp_id)){
        printf("echec creation du tampon etats_camions\n");
        return 1;
    }
    e_camions = (camion_t*)shmat(*tamp_id, NULL, 0);
    
    *file_msg_id = msgget((key_t)FileMsgKEY, 0666 | IPC_CREAT);
    if(!(*file_msg_id)){
        printf("echec creation de la file de messages Faffec\n");
        return 1;
    }

    for(int i = 0 ; i < m ; i++){
        
    }
}

void Camion(int id){
    exit(200 + id);
}
void Controlleur(){
    exit(200);
}

int main(int argc, char* argv[]){
    int tampid, file_msg_id;
    int m = 20;
    int n = 5;
    int cp = 300;
    int *dist_decharge = NULL;
    int *dist_poubelles = NULL;
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

    return EXIT_SUCCESS;
}