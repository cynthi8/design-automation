#
# In order to execute this "Makefile" just type "make"
#	A. Delis (ad@di.uoa.gr)
#

OBJS	= KL_algorithm.o
SOURCE	= KL_algorithm.cpp
HEADER	= 
OUT	= klprog
CC	 = g++
FLAGS	 = -O3 -std=c++17 -g -c -Wall
LFLAGS	 = -lpthread -lm
# -g option enables debugging mode 
# -c flag generates object code for separate files


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)


# create/compile the individual files >>separately<<
KL_algorithm.o: KL_algorithm.cpp
	$(CC) $(FLAGS) KL_algorithm.cpp -lcunit -lcppunit


# clean house
clean:
	rm -f $(OBJS) $(OUT)

