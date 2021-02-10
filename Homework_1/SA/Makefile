CC=g++
CFLAGS=-I.
DEPS = SA_algorithm.hpp

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

saprog: saprog.o SA_algorithm.o
	$(CC) -o saprog saprog.o SA_algorithm.o

clean:
	rm -f saprog saprog.o SA_algorithm.o