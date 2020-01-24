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
/* #include <sys/msg.h> */ /*commento perchè se no mi dà errore quando definisco la struct msgbuf */
#include <unistd.h>
#include <signal.h>
#include "header.h"


int main(int argc, char * argv[], char * envp[]){
	int m_id, s_id, queue_id, num_bytes;
	int i, j;
	struct memoria_condivisa * scacchiera;
	pid_t my_pid;
	
	/*capire se così le prende se no bisogna passarle come argomento alla execv dentro master*/
	SO_MAX_TIME = atoi(envp[0]);
	SO_NUM_G = atoi(envp[1]);
	SO_BASE = atoi(envp[2]);
	SO_ALTEZZA = atoi(envp[3]);
	SO_NUM_P = atoi(envp[4]);
	SO_FLAG_MIN = atoi(envp[5]);
	SO_FLAG_MAX = atoi(envp[6]);
	SO_ROUND_SCORE = atoi(envp[7]);
	SO_N_MOVES = atoi(envp[8]);
	SO_MIN_HOLD_NSEC = atoi(envp[9]);
	
	struct sembuf sops;
	
	/* Getting the IDs of IPC object from command line */
	m_id = atoi(argv[1]);
	s_id = atoi(argv[2]);
	
	/* Attacco la memoria condivisa creata dal processo master */
	scacchiera = shmat(m_id, NULL, 0);
	TEST_ERROR;

	my_pid = getpid();
	
	while(1)
	{
		
	}
	
	/* 
	 * All child  processes are  attached. Then the  shared memory
	 * can be  marked for deletion.  Remember: it will  be deleted
	 * only when all processes are detached from it!!
	 */
	shmctl(m_id, IPC_RMID, NULL);
	
	printf("[PEDINA %5d] Attesa del semaforo ID_PLAY \n", getpid());
		reserveSem(s_id, ID_PLAY);
		printf("[PEDINA %5d] Ottenuto semaforo ID_PLAY \n", getpid());

	printf("[PEDINA %5d] Devo leggere la coda di mio padre %5d \n", getpid(), getppid());
		queue_id = msgget(getppid(), IPC_CREAT | 0600);
		TEST_ERROR;	
		
		while((num_bytes = msgrcv(queue_id, &my_msg, SO_BASE*SO_ALTEZZA, 1, IPC_NOWAIT)) != -1){
			if(num_bytes >= 0)
				printf("[PEDINA %5d] MESSAGGIO RICEVUTO = %s \n", getpid(), my_msg.mtext);
			else
				printf("[PEDINA %5d] ERRORE IN RICEZIONE %d \n", getpid(), errno);
			/* ciclo infinito in attesa che il master inizi la partita*/
		}
	
	
	exit(0);
}
