# CC=g++ -march=native -w -O3
CC=g++ -O3 --std=c++0x -fopenmp
# CC=g++

file = file.o file.h
common = common.h debug.h graph.h
myG = myG.o myG.h

all: insert

insert: main.o $(common) $(file) $(myG)
	$(CC)  -rdynamic -o insert.out main.o file.o myG.o
	
main.o: main.cpp $(common) file.h myG.h
	$(CC) -rdynamic -c main.cpp

file.o: file.cpp $(common) file.h myG.h
	$(CC) -rdynamic -c file.cpp
	
myG.o: myG.cpp myG.h $(common)
	$(CC) -rdynamic -c myG.cpp
	
clean:
	rm *.o