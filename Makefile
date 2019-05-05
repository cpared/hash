# MAKE DE HASH
OBJS =  main.c hash.c hash_pruebas.c testing.c
TIME = tiempos_volumen.sh
EXEC = pruebas
CC = gcc
CFLAGS = -g -std=c99 -Wall -Wconversion -Wtype-limits -pedantic -Werror
VALGRIND = valgrind --leak-check=full --track-origins=yes --show-reachable=yes

all: main
	
main: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) *.c
	$(VALGRIND) ./$(EXEC)
	chmod +x $(TIME)
	./$(TIME) ./$(EXEC)

clean:
	rm -f $(EXEC)

.PHONY: clean main