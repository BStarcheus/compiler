C = clang
CC = clang++
FLAGS = -std=c++11 -g

SRC = $(wildcard src/*.cpp)

LLVM = `llvm-config --cxxflags --ldflags --libs --system-libs`

compiler: $(SRC) runtime
	$(CC) $(FLAGS) $(SRC) -o compiler $(LLVM)

runtime: src/runtime.c
	$(C) -c src/runtime.c -o runtime.o

test: compiler
	./test/runtests.sh

clean:
	rm compiler runtime.o out.s