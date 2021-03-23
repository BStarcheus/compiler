CC = clang++
FLAGS = -std=c++11 -g

SRC = $(wildcard src/*.cpp)

LLVM = `llvm-config --cxxflags --ldflags --libs --system-libs`

compiler: $(SRC)
	$(CC) $(FLAGS) $(SRC) -o compiler $(LLVM)

parsetest: compiler
	./test/runtests.sh

clean:
	rm compiler