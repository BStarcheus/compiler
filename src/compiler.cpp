#include "parser.h"
#include "symbolTable.h"
#include <iostream>

// Global
SymbolTable symb;

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

    Scanner s(dbg);
    if (!s.openFile(argv[1])) {
        std::cout << "Error: File couldn't be opened" << std::endl;
        return 1;
    } 

    Parser p(&s, dbg);
    bool success = p.parse();

    if (dbg) {
        std::cout << "Parse success: " << success << std::endl;

        std::cout << std::endl << std::endl << "Symbol Table:" << std::endl;
        // Check symbol table entries
        for (SymTabMap::iterator it = symb.begin(); it != symb.end(); ++it) {
            std::cout << getTokenTypeName(it->second) << " "\
            << it->second.val << std::endl;
        }
    }

    return 0;
}