#include "parser.h"
#include "symbolTable.h"
#include <iostream>

// Global
SymbolTable symb;

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

    Parser p(&s, dbg);
    bool success = p.parse();
    std::cout << "Parse success: " << success << std::endl;

    std::cout << std::endl << std::endl << "Symbol Table:" << std::endl;
    // Check symbol table entries
    for (SymTabMap::iterator it = symb.begin(); it != symb.end(); ++it) {
        std::cout << getTokenTypeName(it->second) << " " << it->second.val << std::endl;
    }

    return 0;
}