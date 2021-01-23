CC = clang++
FLAGS = -std=c++11

SRC = $(wildcard src/*.cpp)

compiler: $(SRC)
	$(CC) $(SRC) -o compiler

clean:
	rm compiler