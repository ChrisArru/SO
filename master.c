#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/msg.h>
#include <unistd.h>
#include "header.h"
#define GIOCATORE "giocatore"




/*  alarm(SO_MAX_TIME) da mettere all'inizio del round,
    impostare il signal handler per la stampa della matrice
    e la deallocazione della stessa. */


int main(){
	unsigend int count_round = 0;
	pid_t value;
	int sem_id;
	int m_id, i;
	int SO_MAX_TIME = atoi(getenv("SO_MAX_TIME");
	int SO_NUM_G = atoi(getenv("SO_NUM_G")); /*da definire funzione e metodo x make*/
	int SO_BASE = atoi(getenv("SO_BASE"));
	int SO_ALTEZZA = atoi(getenv("SO_ALTEZZA"));
	char * args[/*da vedere*/] = {GIOCATORE};
	struct memoria_condivisa * scacchiera;
	
	
	sem_id = semget(IPC_CREATE, SO_BASE * SO_ALTEZZA, 0/* da vedere */);
	TEST_ERROR;	
	reset_sem();
	m_id = shmget(IPC_CREATE, sizeof(* scacchiera));
	TEST_ERROR;
	scacchiera = shmat(m_id, NULL, 0);
	scacchiera->indice = 0;
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
	/*aspetta che tutti i figli mettano pedine con wait */
        /*deallocazione dei semafori*/
	semctl ( int s_id , /* ignored */ , IPC_RMID );


printf("%i\n", SO_NUM_G);
}


void reset_sem(){
	/*qui dovremmo settare tutti i semafori della scacchiera a 0
	  facendo tipo for(i = 0; i < SO_BASE * SO_ALTEZZA; i++)
	                  semctl(sem_id, scacchiera[i], SETVAL, 0);
        */

}


