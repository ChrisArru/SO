#da sistemare in base ai file che verranno prodotti
CFLAGS=-std=c89 -Wpedantic
TARGET=master
OBJ=master.c


$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)


all: 
	$(export  SO_NUM_G=2)
	$(export SO_NUM_P=10)
	$(export SO_MAX_TIME=3)
	$(export SO_BASE=60)
	$(export SO_ALTEZZA=20)
	$(export SO_FLAG_MIN=5)
	$(export SO_FLAG_MAX=5)
	$(export SO_ROUND_SCORE=10)
	$(export SO_N_MOVES=20)
	$(export SO_MIN_HOLD_NSEC=100000000)
	$(TARGET)


clean:	
	rm -f *.o $(TARGET) *~


run: $(TARGET)
	./$(TARGET)

