CC = g++
CFLAGS = -std=c++2a -Wall -Wextra -pedantic -g

.DEFAULT_GOAL := db

db: main.o database.o fileInteraction.o
	$(CC) -o db main.o database.o fileInteraction.o

main.o: main.cpp database.h
	$(CC) $(CFLAGS) -c main.cpp 

database.o: database.cpp database.h
	$(CC) $(CFLAGS) -c database.cpp

fileInteraction.o: fileInteraction.cpp database.h
	$(CC) $(CFLAGS) -c fileInteraction.cpp

clean :
	rm -f db depend.mak *.o *.txt
	rm -r -f *.dSYM