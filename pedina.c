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

int main(int argc, char * argv[], char * envp[]){
	int m_id, s_id;
	int i, j;
	struct memoria_condivisa * scacchiera;
	pid_t my_pid;
	
	/*capire se così le prende se no bisogna passarle come argomento alla execv dentro master*/
	int SO_MAX_TIME = atoi(envp[0]);
	int SO_NUM_G = atoi(envp[1]);
	int SO_BASE = atoi(envp[2]);
	int SO_ALTEZZA = atoi(envp[3]);
	int SO_NUM_P = atoi(envp[4]);
	
	struct sembuf sops;
	
	/* Getting the IDs of IPC object from command line */
	m_id = atoi(argv[1]);
	s_id = atoi(argv[2]);
	
	/* Attacco la memoria condivisa creata dal processo master */
	scacchiera = shmat(m_id, NULL, 0);
	TEST_ERROR;

	my_pid = getpid();
	printf("%5d\n", my_pid);
	/* 
	 * All child  processes are  attached. Then the  shared memory
	 * can be  marked for deletion.  Remember: it will  be deleted
	 * only when all processes are detached from it!!
	 */
	shmctl(m_id, IPC_RMID, NULL);
	
	/* Inform child processes to start writing to the shared mem 
	i processi giocatore devono inserire le pedine UNA ALLA VOLTA
	*/
	
	/*for(;;)
		/*printf("Processo pedina %d in attesa", my_pid);*/
	
	exit(0);
}
