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
	char comando[80];
	long comandi;
	int lunghezza_vet;
	int vet_comandi[80];
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
	int riga = 0;
	int colonna = 0;
	struct sembuf sops;
	

	
	/*inizializzo il vettore dei comandi */
	for(i = 0; i < sizeof(comando)/sizeof(int); i++)
		comando[i] = 0;
	/* Getting the IDs of IPC object from command line */
	m_id = atoi(argv[1]);
	s_id = atoi(argv[2]);
	
	/* Attacco la memoria condivisa creata dal processo master */
	scacchiera = shmat(m_id, NULL, 0);
	TEST_ERROR;

	my_pid = getpid();
	
	/* mi prendo l'indice della riga e l'indice della colonna su dove sono posizionato */
	for(i = 0; i < SO_ALTEZZA; i++){
		for(j = 0; j < SO_BASE; j++){
			if(scacchiera->scacchiera[i][j].pedina_pid == my_pid){
				riga = i;
				colonna = j;
			}
		}
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
		
		while((num_bytes = msgrcv(queue_id, &my_msg, sizeof(my_msg), getpid(), IPC_NOWAIT)) != -1){
			if(num_bytes >= 0){
				printf("[PEDINA %5d] MESSAGGIO RICEVUTO = %s \n", getpid(), my_msg.mtext);
				sprintf(comando, "%s", my_msg.mtext);
				printf("comando è %s\n", comando);
				comandi = atol(comando);
				printf("numero è %ld\n", comandi);
				
			}
			else
				printf("[PEDINA %5d] ERRORE IN RICEZIONE %d \n", getpid(), errno);
			/* ciclo infinito in attesa che il master inizi la partita*/
		}
		
			for(i = 0; i < sizeof(comando)/sizeof(long); i++){
				vet_comandi[i] = comandi % 10;
				comandi / 10;
			}
			lunghezza_vet = i;
			for( i = 0; i < lunghezza_vet; i++){
				printf("%i", vet_comandi[i]);

			}
			

			/* devo gestire adesso tutte le indicazioni che mi sono state inviate dal giocatore */

		#if 1
		
		for(i = lunghezza_vet; i >= 0; i--){
			switch(comando[i]){
				case 1: /* dobbiamo spostare la pedina sopra */
					if(riga == (SO_ALTEZZA - 1))      /*caso in cui la pedina è già sopra */
						               /* la sposto a destra */
						

	
					break;
				case 2:/* dobbiamo spostare la pedina sotto */
				
					


					break;
				case 3: /* dobbiamo spostare la pedina a destra */
					



					break;
				case 4: /* dobbiamo spostare la pedina a sinistra */
					




					break;
				default: /* ci troviamo in caso di errore */
						



					break;
			}
		}

		#endif
	exit(0);
}
