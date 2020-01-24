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
#define PEDINA "pedina"

void handle_signal(int signal);

int main(int argc, char * argv[], char * envp[]){
	int m_id, s_id;
	int i =0, j, status, queue_id, num_bytes;
	struct memoria_condivisa * scacchiera;
	pid_t value, my_pid, child_pid;
	
	/*printf("%s %s \n", envp[0], envp[1]);*/
	/*printf("%d %d \n", atoi(envp[0]), atoi(envp[1]));*/
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
	int pedine_disposte = SO_NUM_P;
	
	
	char * args[5] = {PEDINA};
	/*char * envp[10];*/
	
	char m_id_str[3*sizeof(m_id)+1];
	char s_id_str[3*sizeof(s_id)+1];
	
	/*char s_max_time[3*sizeof(SO_MAX_TIME)+1];
	char s_num_g[3*sizeof(SO_NUM_G)+1];
	char s_base[3*sizeof(SO_BASE)+1];
	char s_altezza[3*sizeof(SO_ALTEZZA)+1];
	char s_num_pedine[3*sizeof(SO_NUM_P)+1];*/
	
	struct sembuf sops;
	struct sigaction sa;
	sigset_t mymask;
	
	sa.sa_handler = handle_signal;
	sa.sa_flags = 0;
	sigemptyset(&mymask);
	sa.sa_mask = mymask;
	sigaction(EAGAIN, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	
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
	
	/*creo una coda di messaggi
	Ci saranno n code quanti sono i giocatori. In questo modo dai processi pedine posso gettare l'id tramite
	il pid del padre. In più ogni pedina sa che un messaggio è per lei o per un'altra pedina tramite il campo mtype (= pid della pedina)
	*/
	queue_id = msgget(getpid(), IPC_CREAT | 0600);
	TEST_ERROR;	
	
	/* sse tutto funziona qui devo vedere i valori settati dal master!!!!
	for(i = 0; i<SO_BASE*SO_ALTEZZA; i++){
		printf("riga %d, colonna %d, pedina occupa cella %d, pid pedina %5d \n", scacchiera->scacchiera[i].riga,
		scacchiera->scacchiera[i].colonna,
		scacchiera->scacchiera[i].pedinaOccupaCella,
		scacchiera->scacchiera[i].pedina = 0);
	}*/
	
	/*Prendo il semaforo del READY, mi servirà dopo*/
	/*printf("[GIOCATORE] Prendo il semaforo del READY \n");
	sops.sem_num = ID_READY;
	sops.sem_op = -1;
	semop(s_id, &sops, 1);*/
	
	/* Preparing command-line arguments for child's execve */
	sprintf(m_id_str, "%d", m_id);
	sprintf(s_id_str, "%d", s_id);
	args[1] = m_id_str;    /* stringa con m_id */
	args[2] = s_id_str;    /* stringa con s_id */
	args[3] = NULL;        /* NULL-terminated */
	
	/*sprintf(s_max_time, "%d", SO_MAX_TIME);
	sprintf(s_num_g, "%d", SO_NUM_G);
	sprintf(s_base, "%d", SO_BASE);
	sprintf(s_altezza, "%d", SO_ALTEZZA);
	sprintf(s_num_pedine, "%d", SO_ALTEZZA);*/
	/*envp[0] = s_max_time;    /* stringa con m_id */
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
	
	scacchiera->rigaRand = rand()%(SO_ALTEZZA);
	scacchiera->colonnaRand = rand()%(SO_BASE);
	
	while(pedine_disposte>0)
	{
	    /*printf("%d \n", semctl(s_id, ID_PEDINE, GETVAL));*/
		printf("[GIOCATORE] %5d Pedine da disporre %d. Riservo semaforo ID_PEDINE \n", my_pid, pedine_disposte);
		reserveSem(s_id, ID_PEDINE);
		/*sops.sem_num = ID_PEDINE;*/
		/*sops.sem_flg = IPC_NOWAIT;*/
		/*sops.sem_op = -1;
		semop(s_id, &sops, 1);*/
		printf("[GIOCATORE] %5d Superato semaforo ID_PEDINE\n", my_pid);
		/*se sono qui la pedina è libera di muoversi perchè ha il semaforo*/
		/*printf("[GIOCATORE] %5d Cella %d %d Occupata ? %d \n", my_pid, scacchiera->rigaRand,scacchiera->colonnaRand,scacchiera->scacchiera[scacchiera->rigaRand][scacchiera->colonnaRand].pedinaOccupaCella);*/
		if (scacchiera->scacchiera[scacchiera->rigaRand][scacchiera->colonnaRand].pedinaOccupaCella == 0 ){
			scacchiera->scacchiera[scacchiera->rigaRand][scacchiera->colonnaRand].pedinaOccupaCella = 1;
			
			/*scacchiera->indice ++;*/
			switch(value = fork()){
			case -1:
				TEST_ERROR;
				break;			
			case 0:
				/*printf("[GIOCATORE] %5d Creato processo pedina %5d \n", my_pid, getpid()); */
				
				child_pid = getpid();
				/*printf("[GIOCATORE] %5d Creato processo pedina %5d \n", my_pid, child_pid);*/
				execve(PEDINA, args, envp);
				TEST_ERROR;
			default:
				break;
			}
			
			/*printf("[GIOCATORE] %5d Pedine disposte %d \n", my_pid, pedine_disposte);*/
			/*getchar();*/
			scacchiera->scacchiera[scacchiera->rigaRand][scacchiera->colonnaRand].pedina = getpid(); /*messo qui e non dentro la fork() prendo il pid del padre*/
			pedine_disposte--;
			printf("[GIOCATORE] %5d Pedine disposte %d su riga %d colonna %d \n", my_pid, pedine_disposte, scacchiera->rigaRand,scacchiera->colonnaRand);
			/*printf("[GIOCATORE] Rilascio semaforo ID_PEDINE \n");*/
		   
		}
		/*printf("[GIOCATORE] %5d scacchiera indice %d \n", my_pid, scacchiera->indice);*/
		/*getchar();*/
		/*scacchiera->indice++;*/
		scacchiera->rigaRand = rand()%(SO_ALTEZZA); /* se metto SO_ALTEZZA+1 nel rand() mi prende anche l'ultima riga ma  i for non ciclano fino a 20 ma fino a 19!!!!!*/
		scacchiera->colonnaRand = rand()%(SO_BASE);/* se metto SO_BASE+1 nel rand() mi prende anche l'ultima riga ma  i for non ciclano fino a 20 ma fino a 19!!!!!*/
		/*getchar();*/
		/*printf("[GIOCATORE] %5d scacchiera indice %d \n", my_pid, scacchiera->indice);
		printf("[GIOCATORE] %5d Rilascio semaforo ID_PEDINE \n", my_pid);*/
		releaseSem(s_id, ID_PEDINE);
		
		/*sops.sem_flg = IPC_NOWAIT;*/
		/*sops.sem_num = ID_PEDINE;
		sops.sem_op = 1;
		semop(s_id, &sops, 1);*/
	}

	
	/*Dico al master (che è in attesa sul semaforo ID_GIOCATORI) che abbiamo finito*/
	sops.sem_num = ID_GIOCATORI;
	sops.sem_op = 1;
	semop(s_id, &sops, 1);
	
	printf("[GIOCATORE] %5d Ho finito!!!! \n", my_pid);
	
	/* 
	 * All child  processes are  attached. Then the  shared memory
	 * can be  marked for deletion.  Remember: it will  be deleted
	 * only when all processes are detached from it!!
	 */
	shmctl(m_id, IPC_RMID, NULL);
	
	
	while(1)
	{
		/* ciclo infinito in attesa che il master inizi la partita
		Una volta ricevuta comunicazione dal master dell'inizio del round devo dare indicazione alle mie pedine
		su come muoversi
		*/
		/*Semaforo che mi indica che il MASTER vuole iniziare un round*/
		printf("[GIOCATORE %5d] Attesa del semaforo ID_ROUND \n", getpid());
		reserveSem(s_id, ID_ROUND);
		printf("[GIOCATORE %5d] Ottenuto semaforo ID_ROUND \n", getpid());
		
		/*uscito da qua devo dare indicazioni alle pedine per il movimento*/
		printf("[GIOCATORE %5d] Attesa del semaforo ID_MOVE \n", getpid());
		reserveSem(s_id, ID_MOVE);
		
		for(i = 0; i<SO_ALTEZZA; i++){
			for(j = 0; j<SO_BASE; j++){
				/*la cella deve essere occupata da una pedina && la pedina è del processo GIOCATORE che attualmente sta ciclando*/
				if((scacchiera->scacchiera[i][j].pedinaOccupaCella == 1) && (scacchiera->scacchiera[i][j].pedina == getpid())){
					/*la pedina è mia, devo dargli delle indicazioni*/
					my_msg.mtype = 1; /*deve essere >0. Posso forzarlo al pid della pedina per determinare quale processo deve leggere il messaggio */
					num_bytes = sprintf(my_msg.mtext, "ciao %d", i);
					num_bytes++; /*bisogna tener conto del "/0" finale per terminazione stringa */
			
					msgsnd(queue_id, &my_msg, num_bytes, 0); /*invio il messaggio*/
					
					printf("[GIOCATORE %5d]Ho scritto %s a mio figlio %5d \n", getpid(), &my_msg.mtext, child_pid);
					TEST_ERROR;
				}
			}
		}
		
		releaseSem(s_id, ID_MOVE);
		printf("[GIOCATORE %5d] Ottenuto semaforo ID_MOVE \n", getpid());
		releaseSem(s_id, ID_READY_TO_PLAY);
	}
		
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
		case SIGUSR1:
			break;
		case SIGUSR2:
			break;
	}
}