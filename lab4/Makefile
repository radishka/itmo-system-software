CC=gcc

CFLAGS=-Wall -Wextra --std=gnu99 -pedantic -Werror
OBJ=$(SRC:.c=.o)

all: head xargs

head: head.o
	$(CC) head.o -o head

xargs: xargs.o
	$(CC) xargs.o -o xargs

head.o: head.c
	$(CC) $(CFLAGS) -c ./head.c

xargs.o: xargs.c
	$(CC) $(CFLAGS) -c ./xargs.c

clean:
	rm -rf head xargs *.o
