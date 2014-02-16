
CC=gcc
LD=gcc
CFLAGS=-O2 -Wall
LDFLAGS=-lX11

OBJECTS=jload.o

all: jload

jload: $(OBJECTS)
	$(LD) -o jload $(OBJECTS) $(LDFLAGS)
	strip jload

.c.o: $*.c
	$(CC) -c $(CFLAGS) $*.c


clean:
	rm -f *.o jload
