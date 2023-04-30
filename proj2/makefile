CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic

all: proj2

proj2: proj2.c proj2.h
	$(CC) $(CFLAGS) proj2.c -o proj2

clean: 
	rm -f proj2 proj2.out

zip: proj2.c proj2.h makefile
	zip proj2.zip proj2.c proj2.h makefile

run: proj2
	./proj2 2 2 10 10 10

valgrind: proj2
	valgrind --leak-check=full --show-leak-kinds=all ./proj2 2 2 10 10 10



