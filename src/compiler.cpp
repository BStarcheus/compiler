#include "scanner.h"
#include "symbolTable.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Error: Missing positional argument filename" << std::endl;
        return 1;
    }

    // TODO add debug flag argument
    bool dbg = true;

    Scanner s(dbg);
    if (!s.openFile(argv[1])) {
        std::cout << "Error: File couldn't be opened" << std::endl;
        return 1;
    } 

    Token token;
    do {
        token = s.scan();
    } while (token.type != T_EOF);

    return 0;
}