#include <stdio.h>
#include <stdlib.h>

#define TEST_ERROR    if (errno) {dprintf(STDERR_FILENO,		\
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}



/*
DA QUI CREO DELLE COSTANTI PER GLI INDICI DEI SEMAFORI
NOTA BENE: i semafori devono essere presenti in ogni cella quindi ci saranno SO_ALTEZZA*SO_BASE semafori
In più serviranno dei semafori per sincronizzare i vari processi, creo 5000 semafori e dal 4800 (escluso) serviranno
per la gestione dei processi
*/
#define ID_READY         4801 /*Semaforo per segnalare che i processi figli sono pronti*/
#define ID_GIOCATORI 4802 /*Semaforo che indica che i giocatori hanno disposto le pedine*/
#define ID_PEDINE        4803 /*Semaforo che indica che i giocatori hanno disposto le pedine*/
#define ID_ROUND        4804 /*Semaforo che indica che master vuole iniziare un nuovo round*/
#define ID_MOVE          4805 /*Semaforo che indica che UN giocatore ha finito di dare le indicazioni alle pedine */
#define ID_READY_TO_PLAY          4806 /*Semaforo che indica che TUTTI i giocatori hanno finito di dare le indicazioni e informa MASTER che può iniziare il timer*/
#define ID_PLAY          4807 /*Master dà inizio alla partita e pedine si muovono*/


/* Global variables over the BSS*/
int SO_MAX_TIME;
int SO_NUM_G;
int SO_BASE;
int SO_ALTEZZA; 
int SO_NUM_P;
int SO_FLAG_MIN;
int SO_FLAG_MAX;
int SO_ROUND_SCORE;
int SO_N_MOVES;
int SO_MIN_HOLD_NSEC;

typedef unsigned int bandierina; /*== 0 cella non ha bandierina else ha valore dato dal master */

typedef struct cella{
	int riga, colonna;	
	bandierina bandierina;
	pid_t pedina;
	pid_t pedina_pid;
	int pedinaOccupaCella;
} cella;

typedef struct memoria_condivisa{
	unsigned int rigaRand;
	unsigned int colonnaRand;
	/*cella scacchiera[4800];*/
	cella scacchiera[40][120]; /*matrice con SO_ALTEZZA righe e SO_BASE colonne*/
	pid_t giocatori[4];
	unsigned int punteggio[4];
	unsigned int mosse[4];
	unsigned int posizionePedina[1600];
	unsigned int posBandierine[40][2];
	unsigned int numero_round;
	unsigned int numero_bandierine;
	pid_t pid_master;
}memoria_condivisa;

struct msgbuf{
	long mtype;
	char mtext[4800];
};
struct msgbuf my_msg; /* struttura per la coda dei messaggi*/
struct msgbuf my_msg_bandierine;
/*scacchiera= calloc(SO_BASE * SO_ALTEZZA, sizeof(* scacchiera)); /*free finale*/


/*
struct cella
{
    int cellaOccupata;
    pid_t pedina;
	bandierina bandierina;
};

struct memoria_condivisa 
{
    int row;
    int col;
    float percentage;
    // structure within structure
    struct cella cell;
};
*/

/*
 * The following union must be defined as required by the semctl man
 * page
 */
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
				    (Linux-specific) */
};

/*int initSemAvailable(int semId, int semNum);
int initSemInUse(int semId, int semNum);
int reserveSem(int semId, int semNum);
int releaseSem(int semId, int semNum);*/

/* Initialize semaphore to 1 (i.e., "available")*/
int initSemAvailable(int semId, int semNum) {
	union semun arg;
	arg.val = 1;
	return semctl(semId, semNum, SETVAL, arg);
}
/* Initialize semaphore to 0 (i.e., "in use")*/
int initSemInUse(int semId, int semNum) {
	union semun arg;
	arg.val = 0;
	return semctl(semId, semNum, SETVAL, arg);
}

/*Reserve semaphore - decrement it by 1*/
int reserveSem(int semId, int semNum) {
	struct sembuf sops;
	sops.sem_num = semNum;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semop(semId, &sops, 1);
}
/*Release semaphore - increment it by 1*/
int releaseSem(int semId, int semNum) {
	struct sembuf sops;
	sops.sem_num = semNum;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	return semop(semId, &sops, 1);
}
