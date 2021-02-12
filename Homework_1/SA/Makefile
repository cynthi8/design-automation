CC=g++
CFLAGS=-I. -g -Wall -Werror
DEPS = SA_algorithm.hpp

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: saprog satest

saprog: saprog.o SA_algorithm.o
	$(CC) -o saprog saprog.o SA_algorithm.o

satest: satest.o SA_algorithm.o
	$(CC) -o satest satest.o SA_algorithm.o

clean:
	rm -f satest saprog *.o