#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/msg.h>
#include <unistd.h>
#include "header.h"
#define GIOCATORE "giocatore"

//DA QUI CREO DELLE COSTANTI PER GLI INDICI DEI SEMAFORI
#define ID_READY     0 // Semaforo per segnalare che i processi figli sono pronti
#define ID_GIOCATORI 1 // Semaforo per mettere in attesa il master che i giocatori facciano il loro compito
#define ID_PEDINE    2 // Semaforo per mettere in attesa il master che i giocatori facciano il loro compito




/*  alarm(SO_MAX_TIME) da mettere all'inizio del round,
    impostare il signal handler per la stampa della matrice
    e la deallocazione della stessa. */


int main(){
	unsigned int count_round = 0;
	pid_t value, child_pid;
	int sem_id;
	int m_id, i;
	int SO_MAX_TIME = atoi(getenv("SO_MAX_TIME");
	int SO_NUM_G = atoi(getenv("SO_NUM_G")); /*da definire funzione e metodo x make*/
	int SO_BASE = atoi(getenv("SO_BASE"));
	int SO_ALTEZZA = atoi(getenv("SO_ALTEZZA"));
	char * args[4/*da vedere*/] = {GIOCATORE};
	struct memoria_condivisa * scacchiera;
	char m_id_str[3*sizeof(m_id)+1];
	char s_id_str[3*sizeof(sem_id)+1];
	struct sembuf sops;
	
	/* Initialize the common fields */
	sops.sem_num = 0;     /* check the 0-th semaphore */
	sops.sem_flg = 0;     /* no flag */
	
	sem_id = semget(IPC_CREATE, SO_BASE * SO_ALTEZZA, 0/* da vedere */);
	TEST_ERROR;	
	reset_sem(sem_id);
	m_id = shmget(IPC_CREATE, sizeof(* scacchiera)); //creo la memoria condivisa
	TEST_ERROR;
	scacchiera = shmat(m_id, NULL, 0); //attach memoria virtuale a "master"
	scacchiera->indice = 0;
	
	/* Preparing command-line arguments for child's execve */
	sprintf(m_id_str, "%d", m_id);
	sprintf(s_id_str, "%d", sem_id);
	args[1] = m_id_str;    /* stringa con m_id */
	args[2] = s_id_str;    /* stringa con sem_id */
	args[3] = NULL;        /* NULL-terminated */
	
	value = malloc(SO_NUM_G*sizeof(*value));
	for(i = 0; i < SO_NUM_G; i++){
		switch(value = fork()){
			case -1:
				TEST_ERROR;
				break;			
			case 0:
				execve(GIOCATORE, args, NULL);
				TEST_ERROR;
			default:
				break;
		}
	}
	
	/* 
	 * All child  processes are  attached. Then the  shared memory
	 * can be  marked for deletion.  Remember: it will  be deleted
	 * only when all processes are detached from it!!
	 */
	shmctl(m_id, IPC_RMID, NULL);
	
	/* Inform child processes to start writing to the shared mem */
	sops.sem_num = ID_READY;
	sops.sem_op = SO_NUM_G;
	semop(sem_id, &sops, 1);
	
	//Rimango in attesa finchè giocatori non finisce di mettere le pedine
	sops.sem_num = ID_GIOCATORI;
	sops.sem_op = -1;
	semop(sem_id, &sops, 1);
	
	printf("Giocatori hanno finito di mettere le pedine");
	
	
	/*aspetta che tutti i figli mettano pedine con wait */
        /*deallocazione dei semafori*/
	semctl ( sem_id , ID_READY , IPC_RMID );


	printf("%i\n", SO_NUM_G);
	
	exit(0);
}


void reset_sem(int sem_id){
	/*qui dovremmo settare tutti i semafori della scacchiera a 0
	  facendo tipo for(i = 0; i < SO_BASE * SO_ALTEZZA; i++)
	                  semctl(sem_id, scacchiera[i], SETVAL, 0);
        */
	for(i = 0; i < SO_BASE * SO_ALTEZZA; i++)
		initSemInUse(sem_id, i);
}


