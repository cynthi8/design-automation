CC=g++
CFLAGS=-I.
DEPS =

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

klprog: KL_algorithm.o
	$(CC) -o klprog KL_algorithm.o

clean:
	rm -f klprog KL_algorithm.o