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
#define GESTISCI_RISORSE_AND_CHECK(sem_id, sops, num)			\
	if (semop(sem_id, sops, num) == -1) {				\
	        switch (errno) {					\
		case EIDRM:						\
			printf("PID = %d, riga:%d : semaforo rimosso mentre ero bloccato\n", \
			       getpid(), __LINE__);			\
			exit(EXIT_FAILURE);				\
		case EINVAL:						\
			printf("PID = %d, riga:%d : semaforo rimosso prima di blocco (o mai esistito)\n", \
			       getpid(), __LINE__);			\
			exit(EXIT_FAILURE);				\
		case EAGAIN:						\
			printf("PID = %d, riga:%d : mi sono stufato di aspettare\n", \
			       getpid(), __LINE__);			\
			exit(EXIT_FAILURE);				\
						\
		default:						\
			printf("PID = %d, riga:%d : altro errore\n",	\
			       getpid(), __LINE__);			\
			TEST_ERROR;					\
			exit(EXIT_FAILURE);				\
		}							\
	}
	

int main(int argc, char * argv[], char * envp[]){
	int m_id, s_id, queue_id, num_bytes;
	int i, j, riga_pedina, colonna_pedina, mosse_residue;
	struct memoria_condivisa * scacchiera;
	pid_t my_pid;
	char comando[80];
	unsigned long comandi;
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
	struct timespec my_time;	
	bandierina punteggio = 0;
	signed int distanza_colonne, distanza_righe;
	char *token;
	
	/*setto il tempo per la nanosleep */
	my_time.tv_sec = 0;
	my_time.tv_nsec = SO_MIN_HOLD_NSEC;

	
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
	


	printf("[PEDINA %5d] Attesa del semaforo ID_PLAY \n", getpid());
	reserveSem(s_id, ID_PLAY);
	printf("[PEDINA %5d] Ottenuto semaforo ID_PLAY \n", getpid());
	


	/* da qui si muovono le pedine */
     int nretry = 1000;
	int moveY = 1;
	int moveX = 1;
	if(distanza_righe > 0){
		moveY = -1;
	}
	
	if(distanza_colonne > 0){
		moveX = -1;
	}
	
	while(mosse_residue)
	{
		for(i=0; i<abs(distanza_righe); i++)
		{
			reserveSem(s_id, ((riga_pedina + moveY) * SO_BASE) + colonna_pedina);
			if (scacchiera[riga_pedina + moveY][colonna_pedina].pedinaOccupaCella == 1){
				
				//cella occupata;
				nretry--; //variabile da dichiarare all'inio di pedina 
				if (nretry == 0) break; // qui dovrei alzare il signal per il processo giocatore
				i--; //decremento al variabile del cicli for per rieseguire la stessa mossa
			}
			else
			{
				
			
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 0;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = 0;
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = 0;
				releaseSem(s_id, (riga_pedina) * SO_BASE) + colonna_pedina);
				nanosleep (& my_time , NULL );
				riga_pedina = riga_pedina + moveY; 
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 1;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = getpid();
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = getppid();
				if(scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina){
					/* aggiornare posizione bandierine sul vettore presente in scacchiera */
					/*prendere punteggio */
					scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina = 0;
					scacchiera -> scacchiera.numero_bandierine--;
				}
				
				mosse_residue--;
			
				if (mosse_residue == 0) break;
			}
		}
	}
		
	while(mosse_residue)
	{
		for(j=0; j<abs(distanza_colonne); j++)
		{
			reserveSem(s_id, ((riga_pedina) * SO_BASE) + colonna_pedina + moveX);
			if (scacchiera[riga_pedina][colonna_pedina + moveX].pedinaOccupaCella == 1){
				
				//cella occupata;
				nretry--; //variabile da dichiarare all'inio di pedina 
				if (nretry == 0) break; // qui dovrei alzare il signal per il processo giocatore
				i--; //decremento al variabile del cicli for per rieseguire la stessa mossa
			}
			else
			{
				
			
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 0;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = 0;
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = 0;
				releaseSem(s_id, (riga_pedina) * SO_BASE) + colonna_pedina);
				nanosleep (& my_time , NULL );
				colonna_pedina= colonna_pedina + moveX; 
				scacchiera->scacchiera[riga_pedina][colonna_pedina].pedinaOccupaCella = 1;  /* da invertire */
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina_pid = getpid();
				scacchiera -> scacchiera[riga_pedina][colonna_pedina].pedina = getppid();
				if(scacchiera -> scacchiera[riga_pedina][colonna_pedina].bandierina){
					/* aggiornare posizione bandierine sul vettore presente in scacchiera */
					/*prendere punteggio */
					scacchiera[riga_pedina][colonna_pedina].bandierina = 0;
					scacchiera -> scacchiera.numero_bandierine--;
				}
				
				mosse_residue--;
				if (mosse_residue == 0) break;
			}
		}
	}
	
	
	
	if(scacchiera -> scacchiera.numero_bandierine == 0){
		kill ( scacchiera->scacchiera.pid_master, SIGUSR1 );
		TEST_ERROR;
	}
	}
	printf("[PEDINA %5d] pedina si trova su riga %d colonna %d mosse residue %d \n", getpid(), riga_pedina, colonna_pedina, mosse_residue);
	
	
	exit(0);
}


void handle_signal(int signal){
	switch(signal){
		case SIGINT:
			/* to be manged */
			break;
		case SIGALRM:
			print_status();
			/* deallocazione della memoria condivisa*/
			/*fine del programma*//* to be manged */
			break;
		case EAGAIN:
			/*printf("Ricevuto segnale EAGAIN \n");*/
			break;
		case SIGUSR1:
				
			break;
		case SIGUSR2:
			break;
	}



























	#if 0
	/* cosa delle pedine si può gestire con il flag EAGAIN */
	
	printf("[PEDINA %5d] Attesa del semaforo ID_PLAY \n", getpid());
		reserveSem(s_id, ID_PLAY);
		printf("[PEDINA %5d] Ottenuto semaforo ID_PLAY \n", getpid());

	printf("[PEDINA %5d] Devo leggere la coda di mio padre %5d \n", getpid(), getppid());
		queue_id = msgget(getppid(), IPC_CREAT | 0600);
		TEST_ERROR;	
		printf("La dimensione è %d", msgrcv(queue_id, &my_msg, sizeof(my_msg), getpid(), IPC_NOWAIT));
		while((num_bytes = msgrcv(queue_id, &my_msg, sizeof(my_msg), getpid(), IPC_NOWAIT)) != -1){
			if(num_bytes >= 0){
				printf("[PEDINA %5d] MESSAGGIO RICEVUTO = %s \n", getpid(), my_msg.mtext);
				sprintf(comandi, "%u", my_msg.mtext);
				#if 0
				printf("[PEDINA] PID: %d comando è %s\n", getpid(), comando);
				comando[20] = '/0';
				comandi = atol(comando);
				#endif
				printf("[PEDINA] PID: %d numero è %lu\n", getpid(), comandi);
				
			}
			#if 1
				else
				printf("[PEDINA %5d] ERRORE IN RICEZIONE %d \n", getpid(), errno);
			/* ciclo infinito in attesa che il master inizi la partita*/
		}
			#endif
			for(i = 0; i < (sizeof(comandi)/sizeof(unsigned long)); i++){
				vet_comandi[i] = comandi % 10;
				comandi / 10;
			}
			lunghezza_vet = i;
			for( i = 0; i < lunghezza_vet; i++){
				printf("%i ookkkkkkkkkkkkkk", vet_comandi[i]);

			}
			/* finisce effettivamente la disposizione dei giocatori alle pedine */
			reserveSem(s_id, ID_PLAY);

			/* devo gestire adesso tutte le indicazioni che mi sono state inviate dal giocatore */

		#if 1
		while(SO_N_MOVES){
			for(i = lunghezza_vet; i >= 0; i--){
				switch(comando[i]){
					case 1: /* dobbiamo spostare la pedina sopra */
						if(riga == (SO_ALTEZZA - 1))      /*caso in cui la pedina è già in cima */
							i++;  /* vado avanti con le istruzioni */
						else
							switch(semctl( s_id , ((riga + 1) * SO_BASE) + colonna, GETVAL )){
								case 0:
									i++; /* da vedere */
									break;
								case 1: /* pedina libera */
									scacchiera->scacchiera[riga][colonna].pedinaOccupaCella = 0;  /* da invertire */
									scacchiera -> scacchiera[riga][colonna].pedina_pid = 0;
									scacchiera -> scacchiera[riga][colonna].pedina = 0;
									sops.sem_num = (riga * SO_BASE) + colonna;
									sops.sem_flg = IPC_NOWAIT;
									sops.sem_op = 1;
									GESTISCI_RISORSE_AND_CHECK(s_id, &sops, 1);
									printf("ok");
									nanosleep (& my_time , NULL );
									sops.sem_num = (riga + 1 * SO_BASE) + colonna;
									sops.sem_flg = IPC_NOWAIT;
									sops.sem_op = -1;
									GESTISCI_RISORSE_AND_CHECK(s_id, &sops, 1);
									scacchiera->scacchiera[riga + 1][colonna].pedinaOccupaCella = 1;
									scacchiera -> scacchiera[riga + 1][colonna].pedina_pid = getpid();
									scacchiera -> scacchiera[riga + 1][colonna].pedina = getppid();
									riga++;
									SO_N_MOVES--;  /* ho spostato la pedina sull' altra cella */
									if(scacchiera -> scacchiera[riga][colonna].bandierina){
										punteggio = punteggio + scacchiera -> scacchiera[riga][colonna].bandierina;
										scacchiera -> scacchiera[riga][colonna].bandierina = 0;
									}
								default:
									break;
							}
						break;
					case 2:/* dobbiamo spostare la pedina sotto */
						if(riga == 0)      /*caso in cui la pedina è già all' estremo inferiore */
							i++;  /* vado avanti con le istruzioni */
						else
							switch(semctl( s_id , ((riga - 1) * SO_BASE) + colonna, GETVAL )){
								case 0:
									i++;
									break;
								case 1: /* pedina libera */
									scacchiera->scacchiera[riga][colonna].pedinaOccupaCella = 0;
									scacchiera -> scacchiera[riga][colonna].pedina_pid = 0;
									scacchiera -> scacchiera[riga][colonna].pedina = 0;
									sops.sem_num = (riga * SO_BASE) + colonna;
									sops.sem_flg = IPC_NOWAIT;
									sops.sem_op = 1;
									GESTISCI_RISORSE_AND_CHECK(s_id, &sops, 1);
									nanosleep (& my_time , NULL );
									sops.sem_num = (riga - 1 * SO_BASE) + colonna;
									sops.sem_flg = IPC_NOWAIT;
									sops.sem_op = -1;
									GESTISCI_RISORSE_AND_CHECK(s_id, &sops, 1);
									scacchiera->scacchiera[riga - 1][colonna].pedinaOccupaCella = 1;
									scacchiera -> scacchiera[riga - 1][colonna].pedina_pid = getpid();
									scacchiera -> scacchiera[riga - 1][colonna].pedina = getppid();
									riga--;
									SO_N_MOVES--;  /* ho spostato la pedina sull' altra cella */
									if(scacchiera -> scacchiera[riga][colonna].bandierina){
										punteggio = punteggio + scacchiera -> scacchiera[riga][colonna].bandierina;
										scacchiera -> scacchiera[riga][colonna].bandierina = 0;
									}
								default:
									break;
							}
						break;
					case 3: /* dobbiamo spostare la pedina a destra */
						if(colonna == (SO_BASE - 1))      /*caso in cui la pedina è già all' estremo destro della cella */
							i++;  /* vado avanti con le istruzioni */
						else
							switch(semctl( s_id , ((riga) * SO_BASE) + colonna + 1, GETVAL )){
								case 0:
									i++;
									break;
								case 1: /* pedina libera */
									scacchiera->scacchiera[riga][colonna].pedinaOccupaCella = 0;
									scacchiera -> scacchiera[riga][colonna].pedina_pid = 0;
									scacchiera -> scacchiera[riga][colonna].pedina = 0;
									sops.sem_num = (riga * SO_BASE) + colonna;
									sops.sem_flg = IPC_NOWAIT;
									sops.sem_op = 1;
									GESTISCI_RISORSE_AND_CHECK(s_id, &sops, 1);
									nanosleep (& my_time , NULL );
									sops.sem_num = (riga * SO_BASE) + colonna + 1;
									sops.sem_flg = IPC_NOWAIT;
									sops.sem_op = -1;
									GESTISCI_RISORSE_AND_CHECK(s_id, &sops, 1);
									scacchiera->scacchiera[riga][colonna+1].pedinaOccupaCella = 1;
									scacchiera -> scacchiera[riga][colonna+1].pedina_pid = getpid();
									scacchiera -> scacchiera[riga][colonna+1].pedina = getppid();
									colonna++;
									SO_N_MOVES--;  /* ho spostato la pedina sull' altra cella */
									if(scacchiera -> scacchiera[riga][colonna].bandierina){
										punteggio = punteggio + scacchiera -> scacchiera[riga][colonna].bandierina;
										scacchiera -> scacchiera[riga][colonna].bandierina = 0;
									}
								default:
									break;
							}
						break;
					case 4: /* dobbiamo spostare la pedina a sinistra */
						if(colonna == 0)      /*caso in cui la pedina è già all' estremo inferiore della cella */
							i++;  /* vado avanti con le istruzioni */
						else
							switch(semctl( s_id , ((riga) * SO_BASE) + colonna - 1, GETVAL )){
								case 0:
									i++;
									break;
								case 1: /* pedina libera */
									scacchiera->scacchiera[riga][colonna].pedinaOccupaCella = 0;
									scacchiera -> scacchiera[riga][colonna].pedina_pid = 0;
									scacchiera -> scacchiera[riga][colonna].pedina = 0;
									sops.sem_num = (riga * SO_BASE) + colonna;
									sops.sem_flg = IPC_NOWAIT;
									sops.sem_op = 1;
									GESTISCI_RISORSE_AND_CHECK(s_id, &sops, 1);
									nanosleep (& my_time , NULL );
									sops.sem_num = (riga * SO_BASE) + colonna - 1;
									sops.sem_flg = IPC_NOWAIT;
									sops.sem_op = -1;
									GESTISCI_RISORSE_AND_CHECK(s_id, &sops, 1);
									scacchiera->scacchiera[riga][colonna-1].pedinaOccupaCella = 1;
									scacchiera -> scacchiera[riga][colonna-1].pedina_pid = getpid();
									scacchiera -> scacchiera[riga][colonna-1].pedina = getppid();
									colonna--;
									SO_N_MOVES--;  /* ho spostato la pedina sull' altra cella */
									if(scacchiera -> scacchiera[riga][colonna].bandierina){
										punteggio = punteggio + scacchiera -> scacchiera[riga][colonna].bandierina;
										scacchiera -> scacchiera[riga][colonna].bandierina = 0;
									}
								default:
									break;
							}




						break;
					default: /* ci troviamo in caso di errore */
						


	
						break;
				}			
			}
		}
					
		
		#endif

	exit(0);
}
#endif
