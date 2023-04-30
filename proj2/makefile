CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
SEMFLAGS = -pthread -lrt

.PHONY: clean all

all: proj2


proj2: proj2.o proj2.h
	$(CC) $(CFLAGS) -o proj2 proj2.o $(SEMFLAGS)

proj2.o: proj2.c proj2.h
	$(CC) $(CFLAGS) -c proj2.c $(SEMFLAGS)

clean:
	rm -f proj2 proj2.out proj2.o 

zip: proj2.c proj2.h makefile
	zip proj2.zip proj2.c proj2.h makefile

run: proj2
	./proj2 2 2 10 10 10

valgrind: proj2
	valgrind --leak-check=full --show-leak-kinds=all ./proj2 2 2 10 10 10



