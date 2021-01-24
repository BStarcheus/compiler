#include "scanner.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;

    Scanner s;
    if (!s.openFile(argv[1])) return 1;

    Token token = Token();

    while (token.type != T_EOF) {
        token = s.scan();
        std::cout << token.type << "  ";
        std::cout << token.val.strVal;
        std::cout << std::endl;
    }

    return 0;
}