CC=gcc
CFLAGS= -O3 -g -Wall -std=gnu11

all: vehicleclient gridserver griddisplay

vehicleclient: vehicleclient.o
	${CC} ${CFLAGS} vehicleclient.c -o vehicleclient

vehicleclient.o: vehicleclient.c
	${CC} ${CFLAGS} -c vehicleclient.c -o vehicleclient.o

gridserver: gridserver.o
	${CC} ${CFLAGS} gridserver.o -o gridserver

gridserver.o: gridserver.c
	${CC} ${CFLAGS} -c gridserver.c -o gridserver.o

griddisplay: griddisplay.o
	${CC} ${CFLAGS} griddisplay.o -o griddisplay

griddisplay.o: griddisplay.c
	${CC} ${CFLAGS} -c griddisplay.c -o griddisplay.o

.PHONY: clean
clean:
	rm -f *.o vehicleclient gridserver griddisplay
