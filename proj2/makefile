CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic

all: proj2.o proj2

proj2.o: proj2.c
	$(CC) $(CFLAGS) proj2.c -o proj2.o

proj2: proj2.o
	$(CC) $(CFLAGS) proj2.c -o proj2