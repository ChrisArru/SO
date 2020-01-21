#include <stdio.h>
#include <stdlib.h>

#define TEST_ERROR    if (errno) {dprintf(STDERR_FILENO,		\
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}



typedef unsigned int bandierina; /*== 0 cella non ha bandierina else ha valore dato dal master */

typedef struct cella{	
	bandierina bandierina;
	pid_t pedina;
	int pedinaOccupaCella;
} cella;

typedef struct memoria_condivisa{
	unsigned long indice;
	/*cella * scacchiera;*/
	cella scacchiera[4800];
	
}memoria_condivisa;

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
int initSemInUse(int semId, int semNum, int tipo) {
	union semun arg;
	arg.val = tipo;
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
