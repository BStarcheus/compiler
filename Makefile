CC = clang++
FLAGS = -std=c++11 -g

SRC = $(wildcard src/*.cpp)

compiler: $(SRC)
	$(CC) $(FLAGS) $(SRC) -o compiler

parsetest: compiler
	./test/runtests.sh

clean:
	rm compiler