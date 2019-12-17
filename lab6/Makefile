CC=gcc
CFLAGS=-Wall

all: server client

server: server.o queue.o
	$(CC) $(CFLAGS) -o server server.o queue.o -lsocket -lnsl

server.o : server.c
	$(CC) $(OPTS) -c server.c 

queue.o : queue.c
	$(CC) $(OPTS) -c queue.c 

client : client.o
	$(CC) $(CFLAGS) -o client client.o -lsocket -lnsl

client.o : client.c
	$(CC) $(OPTS) -c client.c 

clean:
	rm -f *.o server client
