#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/msg.h>
#include <unistd.h>
#include "header.h"
#define PEDINA "pedina"

//DA QUI CREO DELLE COSTANTI PER GLI INDICI DEI SEMAFORI
#define ID_READY     0 // Semaforo per segnalare che i processi figli sono pronti
#define ID_GIOCATORI 1 // Semaforo che indica che i giocatori hanno disposto le pedine
#define ID_PEDINE    2 // Semaforo che indica che i giocatori hanno disposto le pedine

int main(int argc, char * argv[]){
	int m_id, s_id;
	int i, j;
	struct memoria_condivisa * scacchiera;
	pid_t value;
	
	//capire se così le prende se no bisogna passarle come argomento alla execv dentro master
	int SO_MAX_TIME = atoi(getenv("SO_MAX_TIME");
	int SO_NUM_G = atoi(getenv("SO_NUM_G"));
	int SO_NUM_P = atoi(getenv("SO_NUM_P"));
	int SO_BASE = atoi(getenv("SO_BASE"));
	int SO_ALTEZZA = atoi(getenv("SO_ALTEZZA"));
	int pedine_disposte = SO_NUM_P;
	
	char * args[4] = {PEDINA};
	char m_id_str[3*sizeof(m_id)+1];
	char s_id_str[3*sizeof(s_id)+1];
	
	struct sembuf sops;
	
	/* Getting the IDs of IPC object from command line */
	m_id = atoi(argv[1]);
	s_id = atoi(argv[2]);
	
	/* Attacco la memoria condivisa creata dal processo master */
	scacchiera = shmat(m_id, NULL, 0);
	TEST_ERROR;

	my_pid = getpid();
	
	//Prendo il semaforo del READY, mi servirà dopo
	sops.sem_num = ID_READY;
	sops.sem_op = -1;
	semop(sem_id, &sops, 1);
	
	/* Preparing command-line arguments for child's execve */
	sprintf(m_id_str, "%d", m_id);
	sprintf(s_id_str, "%d", s_id);
	args[1] = m_id_str;    /* stringa con m_id */
	args[2] = s_id_str;    /* stringa con s_id */
	args[3] = NULL;        /* NULL-terminated */
	
	/* 
	i processi giocatore devono inserire le pedine UNA ALLA VOLTA
	*/
	while(pedine_disposte<>0)
	{
		reserveSem(s_id, ID_GIOCATORI);
		
		i = rand();
		while(i>SO_BASE*SO_ALTEZZA)
			i = rand();
		//se sono qui la pedina è libera di muoversi perchè ha il semaforo
		if (scacchiera->scacchiera[i].pedinaOccupaCella = 0 ){
			scacchiera->scacchiera[i].pedinaOccupaCella = 1;
			
			switch(value = fork()){
			case -1:
				TEST_ERROR;
				break;			
			case 0:
				scacchiera->scacchiera[i].pedina = value;
				execve(PEDINA, args, NULL);
				TEST_ERROR;
			default:
				break;
			}
			
			pedine_disposte--;
		}
		//
		releaseSem(s_id, ID_GIOCATORI);
	}
	
	//Giocatore ha finito, rilascio il semaforo del READY
	sops.sem_num = ID_READY;
	sops.sem_op = 1;     //ogni giocatore incrementa il semaforo di 1 (deve essere uguale a SO_NUM_G)
	while (sops.sem_op < SO_NUM_G) //tutti i giocatori devono aver finito
		printf("In attesa che tutti i giocatori finiscano di mettere le pedine");
	
	//Dico al master (che è in attesa sul semaforo ID_GIOCATORI) che abbiamo finito
	sops.sem_num = ID_GIOCATORI;
	sops.sem_op = 1;
	semop(sem_id, &sops, 1);
	
	
	
	
	//creo le SO_NUM_P pedine per ogni giocatore
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