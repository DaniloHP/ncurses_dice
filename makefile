output: main.o
	g++ -ltinfo -lncurses main.o

main.o: main.cpp
	g++ -ltinfo -lncurses -c main.cpp