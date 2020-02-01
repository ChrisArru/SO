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
	int m_id, s_id, queue_id, queue_id_bandierine, num_bytes, num_bytes_bandierine;
	int i, j, riga_pedina, colonna_pedina, mosse_residue, indice;
	signed int distanza_colonne, distanza_righe;
	struct memoria_condivisa * scacchiera;
	char* token;
	pid_t my_pid;
	char comando[80];
	unsigned long comandi;
	int lunghezza_vet;
	int vet_comandi[80];
	bandierina punteggio;
	mosse_residue = SO_N_MOVES;
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
	mosse_residue= SO_N_MOVES;
	
	struct sembuf sops;
	struct timespec my_time;
	/* Getting the IDs of IPC object from command line */
	m_id = atoi(argv[1]);
	s_id = atoi(argv[2]);
	
	/* Attacco la memoria condivisa creata dal processo master */
	scacchiera = shmat(m_id, NULL, 0);
	TEST_ERROR;

	my_pid = getpid();
	

	my_time.tv_sec = 0;
	my_time.tv_nsec = SO_MIN_HOLD_NSEC;
	
	for(i = 0; i < 3; i++)
		if(scacchiera ->giocatori[i] == getppid())
			indice = i;

	queue_id_bandierine = msgget(scacchiera->pid_master, IPC_CREAT | 0600);
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
	
	distanza_colonne = 0;
	distanza_righe = 0;
	
	while((num_bytes = msgrcv(queue_id, &my_msg, sizeof(my_msg), getpid(), IPC_NOWAIT)) != -1){
		if(num_bytes >= 0)
		{
			printf("[PEDINA %5d] MESSAGGIO RICEVUTO = %s \n", getpid(), my_msg.mtext);
			
			token = strtok(my_msg.mtext, " ");
			/*printf("Token %s \n", token);*/
			distanza_righe=strtol(token, NULL,10);
			token = strtok(NULL, " ");
			/*printf("Token %s \n", token);*/
			distanza_colonne=strtol(token, NULL,10);
			token = strtok(NULL, " ");
			/*printf("Token %s \n", token);*/
			riga_pedina=strtol(token, NULL,10);
			token = strtok(NULL, " ");
			/*printf("Token %s \n", token);*/
			colonna_pedina=strtol(token, NULL,10);
			
			printf("[PEDINA %5d] Distanza righe %d distanza colonne %d riga pedina %d colonna pedina %d \n", getpid(), distanza_righe, distanza_colonne, riga_pedina, colonna_pedina );
			
		}
		else{
			printf("[PEDINA %5d] ERRORE IN RICEZIONE %d \n", getpid(), errno);
		}
		/* ciclo infinito in attesa che il master inizi la partita*/
	}
	

	while(mosse_residue){                      /*dobbiamo implementare la parte dei SO_MIN_HOLD_NSEC  forse con un'altra nanosleep o con un timer */ 
	printf("PEDINA: è entrato nel ciclo movimento \n");	
	for(i=0; i<abs(distanza_righe); i++){
		if(distanza_righe > 0) {/*distanza riga positiva --> devo SCENDERE*/
			if(reserveSem(s_id, ((riga_pedina -1) * SO_BASE) + colonna_pedina) != -1){
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 0;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = 0;
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = 0;
				releaseSem(s_id, (riga_pedina * SO_BASE) + colonna_pedina);
				printf("Ho spostato sopra la pedina in pos: %u %u in pos:",riga_pedina, colonna_pedina);
				nanosleep (& my_time , NULL );
				riga_pedina--; 
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 1;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = getpid();
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = getppid();
				printf("%u %u\n", riga_pedina, colonna_pedina);
				if(scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina){
					my_msg_bandierine.mtype = scacchiera->pid_master;
					num_bytes_bandierine = sprintf(my_msg_bandierine.mtext, "%5d %d %d %u", getppid(), riga_pedina, colonna_pedina, scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina);
					num_bytes_bandierine++;
					msgsnd(queue_id_bandierine, &my_msg_bandierine, /*sizeof(my_msg)*/num_bytes_bandierine, 0);
					printf("[PEDINA] Ho mandato il messaggio per band = %5d, %u, %u\n", getppid(), riga_pedina, colonna_pedina);
				}
			}
			else
				i++;
		}
			/*gestire tutta la parte dello spostamento */
		else {
			if(reserveSem(s_id, ((riga_pedina +1) * SO_BASE) + colonna_pedina) != -1){
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 0;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = 0;
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = 0;
				releaseSem(s_id, (riga_pedina * SO_BASE) + colonna_pedina);
				printf("Ho spostato sopra la pedina in pos: %u %u in pos:", riga_pedina, colonna_pedina);
				nanosleep (& my_time , NULL );
				riga_pedina++; 
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 1;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = getpid();
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = getppid();
				printf("%u %u\n", riga_pedina, colonna_pedina);	/*distanza riga negativa --> devo SALIRE*/
				if(scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina){
					my_msg_bandierine.mtype = scacchiera->pid_master;
					num_bytes_bandierine = sprintf(my_msg_bandierine.mtext, "%5d %d %d %u", getppid(), riga_pedina, colonna_pedina, scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina);
					num_bytes_bandierine++;
					msgsnd(queue_id_bandierine, &my_msg_bandierine, /*sizeof(my_msg)*/num_bytes_bandierine, 0);
					printf("[PEDINA] Ho mandato il messaggio per band = %u, %u\n",riga_pedina, colonna_pedina);
				}
			}
			else
				i++;
		}
		
		mosse_residue--;
		scacchiera -> mosse[indice]--;

	}
	
	for(j=0; j<abs(distanza_colonne); j++){
		if(distanza_colonne > 0){ /*distanza colonna positiva --> devo andare a SINISTRA*/
			if((reserveSem(s_id, (riga_pedina * SO_BASE) + colonna_pedina - 1)) != - 1){
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 0;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = 0;
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = 0;
				releaseSem(s_id, (riga_pedina * SO_BASE) + colonna_pedina);
				printf("Ho spostato a sinistra la pedina in pos: %u %u in pos:",riga_pedina, colonna_pedina);
				nanosleep (& my_time , NULL );
				colonna_pedina--; 
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 1;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = getpid();
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = getppid();
				printf("%u %u\n", riga_pedina, colonna_pedina);
				if(scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina){
					/* aggiornare posizione bandierine sul vettore presente in scacchiera */
					/*prendere punteggio */
					my_msg_bandierine.mtype = scacchiera->pid_master;
				
					num_bytes_bandierine = sprintf(my_msg_bandierine.mtext, "%5d %d %d %u", getppid(), riga_pedina, colonna_pedina, scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina);
					num_bytes_bandierine++;
					msgsnd(queue_id_bandierine, &my_msg_bandierine, /*sizeof(my_msg)*/num_bytes_bandierine, 0);
					printf("[PEDINA] Ho mandato il messaggio per band = %5d, %u, %u\n", getppid(), riga_pedina, colonna_pedina);
				}
			}
			else j++;
		}
		else    
		{
			if(reserveSem(s_id, (riga_pedina * SO_BASE) + colonna_pedina + 1) != -1){
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 0; 
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = 0;
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = 0;
				releaseSem(s_id, (riga_pedina * SO_BASE) + colonna_pedina);
				printf("Ho spostato a destra la pedina in pos: %u %u in pos:",riga_pedina, colonna_pedina);
				colonna_pedina++; 
				nanosleep (& my_time , NULL );
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 1;  
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = getpid();
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = getppid();
				printf("%u %u\n", riga_pedina, colonna_pedina);
				/*distanza colonna negativa --> devo andare a DESTRA*/
				if(scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina){
					/* aggiornare posizione bandierine sul vettore presente in scacchiera */
					/*prendere punteggio */ /* coda di messaggi da mandare al master per aggiornare il tutto */
					my_msg_bandierine.mtype = scacchiera->pid_master;
					num_bytes_bandierine = sprintf(my_msg_bandierine.mtext, "%5d %d %d %u", getppid(), riga_pedina, colonna_pedina, scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina);
					num_bytes_bandierine++;
					msgsnd(queue_id_bandierine, &my_msg_bandierine, /*sizeof(my_msg)*/num_bytes_bandierine, 0);
					printf("[PEDINA] Ho mandato il messaggio per band = %5d, %u, %u\n", getppid(), riga_pedina, colonna_pedina);
					/*   lo deve fare il master
					scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina = 0;
					scacchiera -> numero_bandierine--;
					*/
				}
			}
			else
				j++;		
		}
		mosse_residue--;
		
	
	}







	#if 0
	for(i=0; i<abs(distanza_righe); i++){
		reserveSem(s_id, riga_pedina+colonna_pedina+i);
		if(distanza_colonne > 0) /*distanza riga positiva --> devo SALIRE*/
			riga_pedina--;
		else
			riga_pedina++;	/*distanza colonna negativa --> devo SCENDERE*/
		mosse_residue--;
		releaseSem(s_id, riga_pedina+colonna_pedina+i);
	}
	
	for(j=0; j<abs(distanza_colonne); j++){
		/*muovo la pedina ma prima devo verificare che il semaforo della cella sia libero*/
		reserveSem(s_id, riga_pedina+colonna_pedina+j);
		/*sops.sem_num = riga_pedina+colonna_pedina+i+j;
		sops.sem_op = -1;
		semop(s_id, &sops, 1);*/
		if(distanza_colonne > 0) /*distanza colonna positiva --> devo andare a SINISTRA*/
			colonna_pedina--;
		else
			colonna_pedina++;	/*distanza colonna negativa --> devo andare a DESTRA*/
		mosse_residue--;
		releaseSem(s_id, riga_pedina+colonna_pedina+j);
	}
	printf("[PEDINA %5d] pedina si trova su riga %d colonna %d mosse residue %d \n", getpid(), riga_pedina, colonna_pedina, mosse_residue);
	
	#endif
	exit(0);
}
}
