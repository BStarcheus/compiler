#include "scanner.h"
#include <iostream>

int main(int argc, char* argv[]) {
    using namespace std;

    if (argc < 2) return 1;

    Scanner s;
    if (!s.openFile(argv[1])) return 1;

    Token token = Token();

    while (token.type != T_EOF) {
        token = s.scan();
        cout << token.type << endl;
    }

    return 0;
}