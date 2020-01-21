#define _GNU_SOURCE  /* Per poter compilare con -std=c89 -pedantic */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/msg.h>
#include <unistd.h>
#include "header.h"
#define GIOCATORE "giocatore"


/*DA QUI CREO DELLE COSTANTI PER GLI INDICI DEI SEMAFORI*/
#define ID_READY     0 /* Semaforo per segnalare che i processi figli sono pronti*/
#define ID_GIOCATORI 1 /* Semaforo per mettere in attesa il master che i giocatori facciano il loro compito*/
#define ID_PEDINE    2 /* Semaforo per mettere in attesa il master che i giocatori facciano il loro compito*/




/*  alarm(SO_MAX_TIME) da mettere all'inizio del round,
    impostare il signal handler per la stampa della matrice
    e la deallocazione della stessa. */

int main(){
	unsigned int count_round = 0;
	pid_t value, child_pid;
	int sem_id;
	int m_id;
	int i;
	int status;
	int riga, colonna;
	int numero_bandierine;
	int SO_MAX_TIME = atoi(getenv("SO_MAX_TIME"));
	int SO_NUM_G = atoi(getenv("SO_NUM_G")); /*da definire funzione e metodo x make*/
	int SO_BASE = atoi(getenv("SO_BASE"));
	int SO_ALTEZZA = atoi(getenv("SO_ALTEZZA"));
	int SO_NUM_P = atoi(getenv("SO_NUM_P"));
	int SO_ROUND_SCORE = atoi(gentenv(SO_ROUND_SCORE));
	int SO_FLAG_MIN = atoi(getenv(SO_FLAG_MIN));
	int SO_FLAG_MAX = atoi(getenv(SO_FLAG_MAX));
	
	char s_max_time[3*sizeof(SO_MAX_TIME)+1];
	char s_num_g[3*sizeof(SO_NUM_G)+1];
	char s_base[3*sizeof(SO_BASE)+1];
	char s_altezza[3*sizeof(SO_ALTEZZA)+1];
	char s_num_pedine[3*sizeof(SO_NUM_P)+1];
	
	char * args[4/*da vedere*/] = {GIOCATORE};
	char * envp[10];
	
	struct memoria_condivisa * scacchiera;
	char m_id_str[3*sizeof(m_id)+1];
	char s_id_str[3*sizeof(sem_id)+1];
	struct sembuf sops;
	
	m_id = shmget(IPC_PRIVATE, sizeof(struct memoria_condivisa), 0600); /*creo la memoria condivisa*/

	TEST_ERROR;
	
	/* Initialize the common fields */
	sops.sem_num = 0;     /* check the 0-th semaphore */
	sops.sem_flg = 0;     /* no flag */

	sem_id = semget(IPC_PRIVATE, SO_BASE * SO_ALTEZZA, 0600/* da vedere */);
	
	TEST_ERROR;	

	reset_sem(sem_id, SO_BASE, SO_ALTEZZA);	
	
	scacchiera = shmat(m_id, NULL, 0); /*attach memoria virtuale a "master"*/
    /*scacchiera->scacchiera = calloc(SO_BASE * SO_ALTEZZA, sizeof(* scacchiera->scacchiera)); /*free finale*/

    /*printf("[MASTER] Scacchiera %d \n", scacchiera->indice);
    printf("[MASTER] Dimensione Scacchiera %d \n", sizeof(*scacchiera));
    printf("[MASTER] Dimensione Scacchiera %d \n", sizeof(*scacchiera->scacchiera));*/
	
	scacchiera->indice = 0;
	initSharedMem(scacchiera->scacchiera, SO_ALTEZZA*SO_BASE);
	/*printf("[MASTER] Pedina occupa cella %d \n", scacchiera->scacchiera[1].pedinaOccupaCella);
	printf("[MASTER] Pedina occupa cella %d \n", scacchiera->scacchiera[1]);*/
	
	/* Preparing command-line arguments for child's execve */
	sprintf(m_id_str, "%d", m_id);
	sprintf(s_id_str, "%d", sem_id);
	args[1] = m_id_str;    /* stringa con m_id */
	args[2] = s_id_str;    /* stringa con sem_id */
	args[3] = NULL;        /* NULL-terminated */
	
	sprintf(s_max_time, "%d", SO_MAX_TIME);
	sprintf(s_num_g, "%d", SO_NUM_G);
	sprintf(s_base, "%d", SO_BASE);
	sprintf(s_altezza, "%d", SO_ALTEZZA);
	sprintf(s_num_pedine, "%d", SO_ALTEZZA);
	envp[0] = s_max_time;    /* stringa con m_id */
	envp[1] = s_num_g;    /* stringa con sem_id */
	envp[2] = s_base;    /* stringa con sem_id */
	envp[3] = s_altezza;    /* stringa con sem_id */
	envp[4] = s_num_pedine;    /* stringa con sem_id */
	envp[5] = NULL;        /* NULL-terminated */
	

	for(i = 0; i < SO_NUM_G; i++){
		switch(value = fork()){
			case -1:
				TEST_ERROR;
				break;			
			case 0:
			    printf("[MASTER] Eseguo processi giocatori \n");
				execve(GIOCATORE, args, envp /*NULL*/);
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
	
	printf("[MASTER] Setto il semaforo ID_READY \n");
	/* Inform child processes to start writing to the shared mem */
	sops.sem_num = ID_READY;
	sops.sem_op = SO_NUM_G;
	semop(sem_id, &sops, 1);
	
	/*Imposto che si può inserire una pedina alla volta*/
	sops.sem_num = ID_PEDINE;
	sops.sem_op = 1;
	semop(sem_id, &sops, 1);
	/*printf("%d \n", semctl(sem_id, ID_PEDINE, GETPID));*/
	
	/*Rimango in attesa finchè giocatori non finisce di mettere le pedine*/
	printf("[MASTER] rimango in attesa su semaforo ID_GIOCATORI \n");
	sops.sem_num = ID_GIOCATORI;
	sops.sem_op = -1;
	semop(sem_id, &sops, 1);
	
	printf("[MASTER] Superato semaforo ID_GIOCATORI \n");
	while(child_pid = wait(&status) != -1){
		dprintf(2, "PID=%d, Sender (PID=%d) status =",
		getpid(),
		child_pid,
		status);
	}
	
	printf("[MASTER] Giocatori hanno finito di mettere le pedine \n");
	
	/* inserisco bandierine */

	numero_bandierine = SO_FLAG_MIN + rand()%((SO_FLAG_MAX - SO_FLAG_MIN)+1);
	for(i = 0; i < numero_bandierine; i++){
		riga = rand()%(SO_BASE+1);
		colonna = rand()%(SO_ALTEZZA+1);
		if((int aus = & access_to(riga, colonna, * scacchiera, SO_BASE).bandierina) != 0) && ((access_to(riga, colonna, * scacchiera, base).pedinaOccupaCella) == 0))
			& int aux = access_to(riga, colonna, * scacchiera).bandiint erina) = SO_ROUND_SCORE /numero_bandierine;
		else
			i--;
	}

/*-------------------------------------------*/


	/*for(i=0; i<SO_ALTEZZA*SO_BASE; i++)
		printf("Pedina %d \n", scacchiera->scacchiera[i].pedina);*/
	/*printf("Scacchiera %d \n", scacchiera->indice);*/
	
	/*aspetta che tutti i figli mettano pedine con wait */
        /*deallocazione dei semafori*/
	semctl ( sem_id , ID_READY , IPC_RMID );


	printf("%i\n", SO_NUM_G);
	
	exit(0);
}



void reset_sem(int sem_id, int base, int altezza){
	/*qui dovremmo settare tutti i semafori della scacchiera a 0
	  facendo tipo for(i = 0; i < SO_BASE * SO_ALTEZZA; i++)
	                  semctl(sem_id, scacchiera[i], SETVAL, 0);
        */
		int i;
	for(i = 0; i < base * altezza; i++)
		initSemInUse(sem_id, i);
}


#if 0
void stampa_scacchiera(memoria_condivisa * mem, int base, int altezza, int base){
	printf("|");
	int i;
	for(i = 0; i < SO_BASE * SO_ALTEZZA; i++){
		if(mem->scacchiera[i]->bandierina > 0)
			printf(" %ui |", mem->scacchiera[i]->bandierina);
		else
			printf("%i", mem -> scacchiera[i]-> pedina);
	}
}
#endif
* cella access_to(int riga, int colonna, memoria_condivisa * mem, int base){
	if( riga == 0 && colonna == 0)
		return scacchiera[0];
	else
		return mem -> scacchiera[((base) * colonna) + riga +1];
}





void initSharedMem(cella * scacchiera, int dim){
	int i;
	for(i = 0; i<dim; i++){
		scacchiera[i].riga = 0;
		scacchiera[i].colonna = 0;
		scacchiera[i].pedinaOccupaCella = 0; /* 0 = cella libera 1 = cella occupata da pedina*/
		scacchiera[i].pedina = 0;
	}
}




