#da sistemare in base ai file che verranno prodotti
CFLAGS=-std=c89 -Wpedantic
#TARGET=master
#OBJ=master.o giocatore.o pedina.o


#$(TARGET): $(OBJ)
#	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)


all: master giocatore pedina
	
export SO_NUM_G=2
	export SO_NUM_P=10
	export SO_MAX_TIME=3
	export SO_BASE=60
	export SO_ALTEZZA=20
	export SO_FLAG_MIN=5
	export SO_FLAG_MAX=5
	export SO_ROUND_SCORE=10
	export SO_N_MOVES=20
	export SO_MIN_HOLD_NSEC=100000000

clean:	
	rm -f *.o master giocatore pedina *~

master: master.c header.h Makefile
	gcc $(CFLAGS)  master.c -o master
	 
giocatore: giocatore.c header.h Makefile
	gcc $(CFLAGS)  giocatore.c -o giocatore
	 
pedina: pedina.c header.h Makefile
	gcc $(CFLAGS)  pedina.c -o pedina
	 
run: all
	./master

