vpath %.c src/
vpath %.h include/
#vpath %.o obj/
vpath server bin/
vpath client bin/

.PHONY: all clean


CC = gcc 
CFLAGS = -Wall -Werror -Wextra -std=gnu99 -g -pthread
O_REP = obj/
B_REP = bin/
L_REP = lib/

PROG_S = server
PROG_C = client


all: $(PROG_S) $(PROG_C)

$(PROG_S): server.o hash_table.o addr_and_hash.o distributed_server.o affichage.o
	$(CC) $(CFLAGS) $(patsubst %,$(O_REP)%,$^) -o $(B_REP)$@

$(PROG_C): client.o hash_table.o addr_and_hash.o affichage.o
	$(CC) $(CFLAGS) $(patsubst %,$(O_REP)%,$^) -o $(B_REP)$@

server.o: server.c client_server.h
client.o: client.c client_server.h
hash_table.o: hash_table.c hash_table.h client_server.h
addr_and_hash.o: addr_and_hash.c addr_and_hash.h client_server.h
affichage.o: affichage.c affichage.h client_server.h
distributed_server.o: distributed_server.c distributed_server.h client_server.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -I include -o $(O_REP)$@

clean:
	rm -f obj/*.o bin/server bin/client
