#define _GNU_SOURCE  /* Per poter compilare con -std=c89 -pedantic */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/msg.h>
#include <unistd.h>
#include "header.h"
#define GIOCATORE "giocatore"





/*DA QUI CREO DELLE COSTANTI PER GLI INDICI DEI SEMAFORI*/
#define ID_READY     0 /*Semaforo per segnalare che i processi figli sono pronti*/
#define ID_GIOCATORI 1 /*Semaforo per mettere in attesa il master che i giocatori facciano il loro compito*/
#define ID_PEDINE    2 /*Semaforo per mettere in attesa il master che i giocatori facciano il loro compito*/




/*  alarm(SO_MAX_TIME) da mettere all'inizio del round,
    impostare il signal handler per la stampa della matrice
    e la deallocazione della stessa. */


int main(){
	printf("SO MAX TIME = %s", getenv("SO_MAX_TIME"));
	printf("SO_NUM_G = %d",atoi(getenv("SO_NUM_G"))); 
	printf("SO_BASE = %d",atoi(getenv("SO_BASE")));
	printf("SO_ALTEZZA = %d",atoi(getenv("SO_ALTEZZA")));
	return 0;
}


