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


typedef struct memoria_condivisa{
	unsigned long indice;
	cella * scacchiera;
	scacchiera= calloc(SO_BASE * SO_ALTEZZA, sizeof(* scacchiera)); /*free finale*/
}memoria_condivisa;


typedef struct cella{
	int riga, colonna;	
	bandierina bandierina;
	pid_t pedina;
	sem_t pedina;
} cella;
