#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/msg.h>
#include <unistd.h>
#include "header.h"
#define PEDINA "pedina"

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
		reserveSem(s_id, 0);
		
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
		
		releaseSem(s_id, 0);
	}
	
	
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
	
	
	
	
	
	//put_pedine_randomly(s_id);
	
	exit(0);
}

//put_pedine_randomly deve inserire le pedine all'interno della scacchiera
//prima che vengano messe le bandierine. Per ora le inserisco in modo random
void put_pedine_randomly(int s_id){
	for(i = 0; i < SO_ALTEZZA; i++)
		for(j = 0; j < SO_BASE; j=j+2)
			initSemInUse(s_id, i+j+(SO_BASE*i));
}