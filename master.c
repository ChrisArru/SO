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
#include "header.h"
#define GIOCATORE "giocatore"


/*  alarm(SO_MAX_TIME) da mettere all'inizio del round,
    impostare il signal handler per la stampa della matrice
    e la deallocazione della stessa. */



struct memoria_condivisa * scacchiera;
struct timespec my_time;

/* Funzione per resettare i semafori */
void reset_sem(int sem_id, int dimensione){
	int i;
	for(i = 0; i < dimensione; i++)
		initSemInUse(sem_id, i);
}

/* Funzione per inizializzare la struttura dati nella memoria condivisa*/
void initSharedMem(memoria_condivisa * scacchiera){
	int i, j;
	scacchiera->rigaRand = 0;
	scacchiera->colonnaRand = 0;
	
	for(i=0; i<SO_NUM_G; i++){
		scacchiera->punteggio[i] = 0;
		scacchiera->mosse[i] = SO_N_MOVES;
	}
	
	for(i = 0; i<SO_ALTEZZA; i++){
		for(j = 0; j<SO_BASE; j++){
			scacchiera->scacchiera[i][j].riga = 0;
			scacchiera->scacchiera[i][j].colonna = 0;
			scacchiera->scacchiera[i][j].pedinaOccupaCella = 0; /* 0 = cella libera 1 = cella occupata da pedina*/
			scacchiera->scacchiera[i][j].pedina = 0;
		}
	}
}

/* Funzione per stampare lo stato della partita*/
void print_status(){
	/*
	Lo stato da stampare comprende:
	- Posizione delle pedine con riferimento a quale dei giocatori appartiene la pedina
	- Posizione delle bandierine (valore)
	- Punteggio attuale giocatori
	- Mosse residue a disposizione
	*/
	int i,j;
	
	for(i=0; i<SO_NUM_G; i++){
		printf("Punteggio giocatore %d \n", scacchiera->punteggio[i]);
		printf("Mosse residue %d \n", scacchiera->mosse[i]);
	}
	
	/*for(i = 0; i<SO_ALTEZZA; i++){
		for(j = 0; j<SO_BASE; j++){
			printf("pedina occupa cella %d, pid pedina %5d \n",
			scacchiera->scacchiera[i][j].pedinaOccupaCella,
			scacchiera->scacchiera[i][j].pedina
			);
		}
	}*/
	
	for(i=0; i<SO_FLAG_MAX; i++)
		printf("Posizione bandierina numero %d riga %d colonna %d", i, scacchiera->posBandierine[i][0], scacchiera->posBandierine[i][1]);
	
	for(i = 0; i<SO_ALTEZZA; i++){
		for(j = 0; j<SO_BASE; j++){
			if(scacchiera->scacchiera[i][j].pedinaOccupaCella == 1){ /*cella occupata da pedina*/
				my_time.tv_sec = 0;
				my_time.tv_nsec = 1	;
				nanosleep(&my_time, NULL);
				if(scacchiera->scacchiera[i][j].pedina == scacchiera->giocatori[0] )
					printf("\033[1;31mA");
				else if(scacchiera->scacchiera[i][j].pedina == scacchiera->giocatori[1] )
					printf("\033[1;33mB");
				else if (scacchiera->scacchiera[i][j].pedina == scacchiera->giocatori[2] )
					printf("\033[1;34mC");
				else if (scacchiera->scacchiera[i][j].pedina == scacchiera->giocatori[3] )
					printf("\033[1;32mD");
				else
					printf("|");
			}
			else if(scacchiera->scacchiera[i][j].bandierina > 0) /* cella occupata da bandierina*/
				printf("\033[1;36m%d", scacchiera->scacchiera[i][j].bandierina );
			else
				printf("0");
			printf("\033[0m"); /*reset color*/
		}
	}
}


int main(){
	unsigned int count_round = 0;
	pid_t value, child_pid;
	int sem_id, queue_id;
	int m_id;
	int i,j, index;
	int status;
	int num_bytes; /*dimensione messaggi per la coda*/
	unsigned int numBandierina, punteggioBandTot, punteggioBandierina;
	
	SO_MAX_TIME = atoi(getenv("SO_MAX_TIME"));
	SO_NUM_G = atoi(getenv("SO_NUM_G")); /*da definire funzione e metodo x make*/
	SO_BASE = atoi(getenv("SO_BASE"));
	SO_ALTEZZA = atoi(getenv("SO_ALTEZZA"));
	SO_NUM_P = atoi(getenv("SO_NUM_P"));
	SO_FLAG_MIN = atoi(getenv("SO_FLAG_MIN"));
	SO_FLAG_MAX = atoi(getenv("SO_FLAG_MAX"));
	SO_ROUND_SCORE = atoi(getenv("SO_ROUND_SCORE"));
	SO_N_MOVES = atoi(getenv("SO_N_MOVES"));
	SO_MIN_HOLD_NSEC = atoi(getenv("SO_MIN_HOLD_NSEC"));
	
	char s_max_time[3*sizeof(SO_MAX_TIME)+1];
	char s_num_g[3*sizeof(SO_NUM_G)+1];
	char s_base[3*sizeof(SO_BASE)+1];
	char s_altezza[3*sizeof(SO_ALTEZZA)+1];
	char s_num_pedine[3*sizeof(SO_NUM_P)+1];
	char s_flag_min[3*sizeof(SO_FLAG_MIN)+1];
	char s_flag_max[3*sizeof(SO_FLAG_MAX)+1];
	char s_round_score[3*sizeof(SO_ROUND_SCORE)+1];
	char s_n_moves[3*sizeof(SO_N_MOVES)+1];
	char s_min_hold_nsec[3*sizeof(SO_MIN_HOLD_NSEC)+1];
	
	char * args[5] = {GIOCATORE};
	char * envp[11];
	
	
	char m_id_str[3*sizeof(m_id)+1];
	char s_id_str[3*sizeof(sem_id)+1];
	struct sembuf sops; /*struttura dei semafori*/
	
	
	m_id = shmget(IPC_PRIVATE, sizeof(struct memoria_condivisa), 0600); /*creo la memoria condivisa*/

	TEST_ERROR;
	
	/* Initialize the common fields */
	sops.sem_num = 0;     /* check the 0-th semaphore */
	sops.sem_flg = 0;     /* no flag */

	sem_id = semget(IPC_PRIVATE, 5000 , 0600);
	TEST_ERROR;	
	
	reset_sem(sem_id, 5000);	
	
	scacchiera = shmat(m_id, NULL, 0); /*attach memoria virtuale a "master"*/
    /*scacchiera->scacchiera = calloc(SO_BASE * SO_ALTEZZA, sizeof(* scacchiera->scacchiera)); /*free finale*/

    /*printf("[MASTER] Scacchiera %d \n", scacchiera->indice);
    printf("[MASTER] Dimensione Scacchiera %d \n", sizeof(*scacchiera));
    printf("[MASTER] Dimensione Scacchiera %d \n", sizeof(*scacchiera->scacchiera));*/
	
	initSharedMem(scacchiera);
	/*printf("[MASTER] Pedina occupa cella %d \n", scacchiera->scacchiera[1].pedinaOccupaCella);
	printf("[MASTER] Pedina occupa cella %d \n", scacchiera->scacchiera[1]);*/
	
	/* Preparing command-line arguments for child's execve */
	sprintf(m_id_str, "%d", m_id);
	sprintf(s_id_str, "%d", sem_id);
	args[1] = m_id_str;    /* stringa con m_id */
	args[2] = s_id_str;    /* stringa con sem_id */
	args[3] = NULL;        /* NULL-terminated */
	
	sprintf(s_max_time, "%d", SO_MAX_TIME);
	sprintf(s_num_g, "%d", SO_NUM_G);
	sprintf(s_base, "%d", SO_BASE);
	sprintf(s_altezza, "%d", SO_ALTEZZA);
	sprintf(s_num_pedine, "%d", SO_NUM_P);	
	sprintf(s_flag_min, "%d", SO_FLAG_MIN);
	sprintf(s_flag_max, "%d", SO_FLAG_MAX);
	sprintf(s_round_score, "%d", SO_ROUND_SCORE);
	sprintf(s_n_moves, "%d", SO_N_MOVES);
	sprintf(s_min_hold_nsec, "%d", SO_MIN_HOLD_NSEC);
	envp[0] = s_max_time;    
	envp[1] = s_num_g;    
	envp[2] = s_base;   
	envp[3] = s_altezza;   
	envp[4] = s_num_pedine;    
	envp[5] = s_flag_min;    
	envp[6] = s_flag_max;    
	envp[7] = s_round_score;    
	envp[8] = s_n_moves;    
	envp[9] = s_min_hold_nsec;    
	envp[10] = NULL;        /* NULL-terminated */
	
	/*value = malloc(SO_NUM_G*sizeof(value));*/

	for(i = 0; i < SO_NUM_G; i++){
		switch(value = fork()){
			case -1:
				TEST_ERROR;
				break;			
			case 0:
			    printf("[MASTER] Eseguo processi giocatori e incremento semaforo ID_READY \n");
				sops.sem_num = ID_READY;
				sops.sem_op = 1;
				semop(sem_id, &sops, 1);
				scacchiera->giocatori[i] = getpid(); /*salvo pid dei giocatori*/
				printf("[MASTER] PID giocatore %5d \n", scacchiera->giocatori[i]);
				execve(GIOCATORE, args, envp /*NULL*/);
				TEST_ERROR;
			default:
				break;
		}
	}
	
	/* 
	 * All child  processes are  attached. Then the  shared memory
	 * can be  marked for deletion.  Remember: it will  be deleted
	 * only when all processes are detached from it!!
	 */
	shmctl(m_id, IPC_RMID, NULL);
	
	printf("[MASTER] Aspetto il semaforo ID_READY \n");
	/* Inform child processes to start writing to the shared mem */
	/* Aspetto che tutti i figli siano nati */
	sops.sem_num = ID_READY;
	sops.sem_op = -SO_NUM_G;
	semop(sem_id, &sops, 1);
	
	/*Imposto che si può inserire una pedina alla volta*/
	sops.sem_num = ID_PEDINE;
	sops.sem_op = 1;
	semop(sem_id, &sops, 1);
	/*printf("%d \n", semctl(sem_id, ID_PEDINE, GETPID));*/
	
	/*Rimango in attesa finchè giocatori non finisce di mettere le pedine*/
	printf("[MASTER] rimango in attesa su semaforo ID_GIOCATORI \n");
	sops.sem_num = ID_GIOCATORI;
	sops.sem_op = -SO_NUM_G;
	semop(sem_id, &sops, 1);
	
	printf("[MASTER] Superato semaforo ID_GIOCATORI \n");
	
	for(i = 0; i<SO_ALTEZZA; i++){
		for(j = 0; j<SO_BASE; j++){
			printf("pedina occupa cella %d, pid pedina %5d riga %d colonna %d \n",
			scacchiera->scacchiera[i][j].pedinaOccupaCella,
			scacchiera->scacchiera[i][j].pedina,
			i,
			j
			);
		}
	}
	
	/*for(i = 0; i<SO_BASE*SO_ALTEZZA; i++){
		printf("pedina occupa cella %d, pid pedina %5d \n",
		scacchiera->scacchiera[i].pedinaOccupaCella,
		scacchiera->scacchiera[i].pedina);
	}*/
	
	printf("[MASTER] Giocatori hanno finito di mettere le pedine \n");

	/*Ora si inizia col ROUND, il master deve piazzare le bandierine*/
	/*Per piazzare le bandierine devo usare celle che non sono occupate da pedine.
	Basterà quindi controllare che scacchiera->scacchiera[scacchiera->indice].pedinaOccupaCella == 0.
	Il numero di bandierine è un numero RANDOM compreso tra SO_FLAG_MIN e SO_FLAG_MAX */
	
	numBandierina = SO_FLAG_MIN + rand()%((SO_FLAG_MAX-SO_FLAG_MIN)+1) ;
	/*getchar();*/
	/*punteggioBandTot = SO_ROUND_SCORE;
	punteggioBandierina = rand()%punteggioBandTot+1;
	punteggioBandTot = punteggioBandTot - punteggioBandierina;*/
	punteggioBandierina = SO_ROUND_SCORE / numBandierina;
	printf("Numero bandierine da disporre %d \n", numBandierina);
	printf("Punteggio bandierina %d \n", punteggioBandierina);
	/*printf("Punteggio bandierine totale rimanente %d \n", punteggioBandTot);*/
	scacchiera->rigaRand = rand()%(SO_ALTEZZA);
	scacchiera->colonnaRand = rand()%(SO_BASE);
	index = 0;
	while(numBandierina>0){
		if(scacchiera->scacchiera[scacchiera->rigaRand][scacchiera->colonnaRand].pedinaOccupaCella == 0){	
			/*posizione libera da pedine*/
			scacchiera->scacchiera[scacchiera->rigaRand][scacchiera->colonnaRand].bandierina = punteggioBandierina;
			/*Mi salvo la posizione della bandierina piazzata.
			La riga indica il numero della bandierina, la colonna conterrà la riga e la colonna della sua posizione*/
			scacchiera->posBandierine[index][0] = scacchiera->rigaRand; 
			scacchiera->posBandierine[index][1] = scacchiera->colonnaRand; 
			index++;
			/*punteggioBandierina = rand()%punteggioBandTot+1;
			if(punteggioBandierina<0)
				punteggioBandierina = 0;
			punteggioBandTot = punteggioBandTot - punteggioBandierina;
			printf("Punteggio bandierina %d \n", punteggioBandierina);
			printf("Punteggio bandierine totale rimanente %d \n", punteggioBandTot);*/
			numBandierina--;
		}
		scacchiera->rigaRand = rand()%(SO_ALTEZZA);
		scacchiera->colonnaRand = rand()%(SO_BASE);
	}

	/*Finito di posizionare le bandierine, stampo lo stato prima di far partire il timer*/
	print_status();
	
	exit(0);
	/* Ora devo iniziare il ROUND*/
	/*alarm(SO_MAX_TIME);*/
	
	/*
	Inizio ciclo infinito dei ROUND.
	Il gioco finisce se timer > SO_MAX_TIME. Nel frattempo il master ciclo e controlla lo stato dei figli
	con wait(). 
	Capire come gestire eventuali casi di errori dovuti alla wait().
	Il round finisce SOLO se TUTTE le bandierine sono state prese. In questo caso si ricomincia un nuovo round con il master che dispone nuovamente le bandierine.
	Se quindi le mosse delle pedine finiscono prima di aver preso tutte le bandierine il gioco finisce (perchè scade il timeout).
	Ogni round comprende:
	- MASTER distribuisce bandierine (dove non ci sono pedine)
	- MASTER stampa stato
	- GIOCATORE dà indicazioni di movimento alle PEDINE
	- MASTER avvia timer (dopo che tutti i giocatori hanno dato le indicazioni di movimento)
	- PEDINE si muovono
	*/
	/* Dico a tutti i giocatori che il round deve iniziare*/
	sops.sem_num = ID_ROUND;
	sops.sem_op = SO_NUM_G;
	semop(sem_id, &sops, 1);
	
	/*Uno alla volta però devono dare disposizioni alle pedine*/
	releaseSem(sem_id, ID_MOVE);
	
	/* Master aspetta che TUTTI i giocatori abbiano finito di dare disposizioni alle pedine*/
	printf("[MASTER] Aspetto su semaforo ID_READY_TO_PLAY \n");
	sops.sem_num = ID_READY_TO_PLAY;
	sops.sem_op = -SO_NUM_G;
	semop(sem_id, &sops, 1);
	printf("[MASTER] Uscito da semaforo ID_READY_TO_PLAY e setto ID_PLAY \n");
	
	
	releaseSem(sem_id, ID_PLAY);
	
	/*exit(0);*/
	
	while(child_pid = wait(&status) != -1){
		dprintf(2, "PID=%d, Sender (PID=%d) status =",
		getpid(),
		child_pid,
		status);
	}
	while(1){
		/*Usare le seguenti funzioni per creare un timer e moniotrare se si ha raggiunto il TIMEOUT
		timer_create()
		timer_settime()
		timer_gettime()
		*/
		
	}
	
	
	
	/*for(i = 0; i<SO_BASE*SO_ALTEZZA; i++){
		printf("pedina occupa cella %d, pid pedina %5d, bandierina %d \n",
		scacchiera->scacchiera[i].pedinaOccupaCella,
		scacchiera->scacchiera[i].pedina,
		scacchiera->scacchiera[i].bandierina
		);
	}*/
	
	
	/* ****************************************************************************** */

	/*aspetta che tutti i figli mettano pedine con wait */
        /*deallocazione dei semafori*/
	semctl ( sem_id , ID_READY , IPC_RMID );
	
	exit(0);
}