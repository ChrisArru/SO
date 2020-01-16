#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/msg.h>
#include <unistd.h>
#include "header.h"

int main(int argc, char * argv[]){
	int m_id, s_id;
	int i, j;
	struct memoria_condivisa * scacchiera;
	
	//capire se così le prende se no bisogna passarle come argomento alla execv dentro master
	int SO_MAX_TIME = atoi(getenv("SO_MAX_TIME");
	int SO_NUM_G = atoi(getenv("SO_NUM_G"));
	int SO_NUM_P = atoi(getenv("SO_NUM_P"));
	int SO_BASE = atoi(getenv("SO_BASE"));
	int SO_ALTEZZA = atoi(getenv("SO_ALTEZZA"));
	int pedine_disposte = SO_NUM_P;
	
	struct sembuf sops;
	
	/* Getting the IDs of IPC object from command line */
	m_id = atoi(argv[1]);
	s_id = atoi(argv[2]);
	
	/* Attacco la memoria condivisa creata dal processo master */
	scacchiera = shmat(m_id, NULL, 0);
	TEST_ERROR;

	my_pid = getpid();
	
	/* 
	 * All child  processes are  attached. Then the  shared memory
	 * can be  marked for deletion.  Remember: it will  be deleted
	 * only when all processes are detached from it!!
	 */
	shmctl(m_id, IPC_RMID, NULL);
	
	/* Inform child processes to start writing to the shared mem 
	i processi giocatore devono inserire le pedine UNA ALLA VOLTA
	*/
	
	while(true)
		printf("Processo pedina %d in attesa", my_pid);
	
	exit(0);
}
