#include "parser.h"
#include <iostream>

int main(int argc, char* argv[]) {
    bool dbg = false;
    std::string filename;

    if (argc < 2) {
        std::cout << "Error: Missing positional argument filename"\
        << std::endl;
        return 1;
    } else if (argc <= 3) {
        for (int i = 2; i < argc; i++) {
            if (!strcmp(argv[i], "-d") || 
                !strcmp(argv[i], "--debug")) {
                dbg = true;
            }
        }
    } else {
        std::cout << "Error: Too many arguments" << std::endl;
        return 1;
    }

    ScopeManager scoper(dbg);
    Scanner scanner(&scoper, dbg);
    Parser parser(&scanner, &scoper, dbg);

    if (!scanner.openFile(argv[1])) {
        std::cout << "Error: File couldn't be opened" << std::endl;
        return 1;
    } 

    bool success = parser.parse();

    if (dbg) {
        std::cout << "Parse success: " << success << std::endl;
    }

    return !success;
}