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
#include <signal.h>
#include "header.h"
#define PEDINA "pedina"

/*DA QUI CREO DELLE COSTANTI PER GLI INDICI DEI SEMAFORI*/
#define ID_READY     0 /*Semaforo per segnalare che i processi figli sono pronti*/
#define ID_GIOCATORI 1 /*Semaforo che indica che i giocatori hanno disposto le pedine*/
#define ID_PEDINE    2 /*Semaforo che indica che i giocatori hanno disposto le pedine*/

void handle_signal(int signal);

int main(int argc, char * argv[], char * envp[]){
	int m_id, s_id;
	int i =0, j, status;
	struct memoria_condivisa * scacchiera;
	pid_t value, my_pid;
	
	/*printf("%s %s \n", envp[0], envp[1]);*/
	/*printf("%d %d \n", atoi(envp[0]), atoi(envp[1]));*/
	/*capire se così le prende se no bisogna passarle come argomento alla execv dentro master*/
	int SO_MAX_TIME = atoi(envp[0]);
	int SO_NUM_G = atoi(envp[1]);
	int SO_BASE = atoi(envp[2]);
	int SO_ALTEZZA = atoi(envp[3]);
	int SO_NUM_P = atoi(envp[4]);
	int pedine_disposte = SO_NUM_P;
	
	
	char * args[4] = {PEDINA};
	/*char * envp[10];*/
	
	char m_id_str[3*sizeof(m_id)+1];
	char s_id_str[3*sizeof(s_id)+1];
	
	char s_max_time[3*sizeof(SO_MAX_TIME)+1];
	char s_num_g[3*sizeof(SO_NUM_G)+1];
	char s_base[3*sizeof(SO_BASE)+1];
	char s_altezza[3*sizeof(SO_ALTEZZA)+1];
	char s_num_pedine[3*sizeof(SO_NUM_P)+1];
	
	struct sembuf sops;
	struct sigaction sa;
	sigset_t mymask;
	
	sa.sa_handler = handle_signal;
	sa.sa_flags = 0;
	sigemptyset(&mymask);
	sa.sa_mask = mymask;
	sigaction(EAGAIN, &sa, NULL);
	
	printf("[GIOCATORE] Processo Giocatori partito \n");
	/*printf("SO__MAX_TIME %d", SO_MAX_TIME);*/
	
	/* Getting the IDs of IPC object from command line */
	m_id = atoi(argv[1]);
	s_id = atoi(argv[2]);
	
	/* Attacco la memoria condivisa creata dal processo master */
	scacchiera = shmat(m_id, NULL, 0);
	TEST_ERROR;
	
	/*scacchiera->scacchiera = calloc(SO_BASE * SO_ALTEZZA, sizeof(* scacchiera->scacchiera)); /*free finale*/
	
	my_pid = getpid();
	
	/*scacchiera->indice = 1;*/
    
	/* sse tutto funziona qui devo vedere i valori settati dal master!!!!
	for(i = 0; i<SO_BASE*SO_ALTEZZA; i++){
		printf("riga %d, colonna %d, pedina occupa cella %d, pid pedina %5d \n", scacchiera->scacchiera[i].riga,
		scacchiera->scacchiera[i].colonna,
		scacchiera->scacchiera[i].pedinaOccupaCella,
		scacchiera->scacchiera[i].pedina = 0);
	}*/
	
	/*Prendo il semaforo del READY, mi servirà dopo*/
	printf("[GIOCATORE] Prendo il semaforo del READY \n");
	sops.sem_num = ID_READY;
	sops.sem_op = 1;
	semop(s_id, &sops, 1);
	
	/* Preparing command-line arguments for child's execve */
	sprintf(m_id_str, "%d", m_id);
	sprintf(s_id_str, "%d", s_id);
	args[1] = m_id_str;    /* stringa con m_id */
	/*envp[0] = s_max_time;    /* stringa con m_id */
	args[2] = s_id_str;    /* stringa con s_id */
	args[3] = NULL;        /* NULL-terminated */
	
	sprintf(s_max_time, "%d", SO_MAX_TIME);
	sprintf(s_num_g, "%d", SO_NUM_G);
	sprintf(s_base, "%d", SO_BASE);
	sprintf(s_altezza, "%d", SO_ALTEZZA);
	sprintf(s_num_pedine, "%d", SO_ALTEZZA);
	/*envp[1] = s_num_g;    /* stringa con sem_id */
	/*envp[2] = s_base;    /* stringa con sem_id */
	/*envp[3] = s_altezza;    /* stringa con sem_id */
	/*envp[4] = s_num_pedine;    /* stringa con sem_id */
	/*envp[5] = NULL;        /* NULL-terminated */
	
	/* 
	i processi giocatore devono inserire le pedine UNA ALLA VOLTA
	*/
	
	printf("[GIOCATORE] Inizio ciclo while per disporre pedine, semaforo ID_PEDINE a 1 dal master \n");
	/*sops.sem_num = ID_PEDINE;
	sops.sem_op = 1;
	semop(s_id, &sops, 1);*/
	printf("GIOCATORE= %d", my_pid);
	while(pedine_disposte>=0)
	{
	    /*printf("%d \n", semctl(s_id, ID_PEDINE, GETVAL));*/
		printf("[GIOCATORE] Pedine da disporre %d. Riservo semaforo ID_PEDINE \n", pedine_disposte);
		
		/*reserveSem(s_id, ID_PEDINE);*/
		sops.sem_num = ID_PEDINE;
		/*sops.sem_flg = IPC_NOWAIT;*/
		sops.sem_op = -1;
		semop(s_id, &sops, 1);
		printf("[GIOCATORE] Superato semaforo ID_PEDINE\n");
		/*se sono qui la pedina è libera di muoversi perchè ha il semaforo*/
		printf("[GIOCATORE] Cella Occupata ? %d \n", scacchiera->scacchiera[scacchiera->indice].pedinaOccupaCella);
		
		if (scacchiera->scacchiera[scacchiera->indice].pedinaOccupaCella == 0 ){
		
		scacchiera->scacchiera[scacchiera->indice].pedinaOccupaCella = 1;
			/*scacchiera->indice ++;*/
			switch(value = fork()){
			case -1:
				TEST_ERROR;
				break;			
			case 0:
				printf("[GIOCATORE] Creato processo pedina %5d", getpid());
				scacchiera->scacchiera[scacchiera->indice].pedina = getpid();
				execve(PEDINA, args, envp);
				printf("pedina eseguita\n");
				TEST_ERROR;
			default:
				break;
			}
			
			pedine_disposte--;
			
			/*printf("[GIOCATORE] Rilascio semaforo ID_PEDINE \n");*/
		   
		}
		
			scacchiera->indice ++;
		releaseSem(s_id, ID_PEDINE);
	}

	/*Giocatore ha finito, rilascio il semaforo del READY*/
	sops.sem_num = ID_READY;
	sops.sem_op = 1;     /*ogni giocatore incrementa il semaforo di 1 (deve essere uguale a SO_NUM_G)*/
	/*while (sops.sem_op < SO_NUM_G) /*tutti i giocatori devono aver finito*/
		/*printf("[GIOCATORE] In attesa che tutti i giocatori finiscano di mettere le pedine");*/
	
	/*Dico al master (che è in attesa sul semaforo ID_GIOCATORI) che abbiamo finito*/
	sops.sem_num = ID_GIOCATORI;
	sops.sem_op = 1;
	semop(s_id, &sops, 1);
	
	for(i = 0; i<SO_BASE*SO_ALTEZZA; i++){
		printf("pedina occupa cella %d, occupa cella= %d pid pedina %5d \n",i ,scacchiera->scacchiera[i].pedinaOccupaCella,
		scacchiera->scacchiera[i].pedina);
	}
	
	
	
	
	/*creo le SO_NUM_P pedine per ogni giocatore*/
	/*for(i = 0; i < SO_NUM_P; i++){
		switch(value = fork()){
			case -1:
				TEST_ERROR;
				break;			
			case 0:
				execve(PEDINA, args, NULL);
				TEST_ERROR;
			default:
				break;
		}
	}*/
	
	/* 
	 * All child  processes are  attached. Then the  shared memory
	 * can be  marked for deletion.  Remember: it will  be deleted
	 * only when all processes are detached from it!!
	 */
	shmctl(m_id, IPC_RMID, NULL);
		
	exit(0);
}

void handle_signal(int signal){
	switch(signal){
		case SIGINT:
			/* to be manged */
			break;
		case SIGALRM:
			/* to be manged */
			break;
		case EAGAIN:
			/*printf("Ricevuto segnale EAGAIN \n");*/
			break;
	}
}
