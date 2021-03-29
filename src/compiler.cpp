#include "parser.h"
#include <iostream>

int main(int argc, char* argv[]) {
    bool dbgScanner = false;
    bool dbgParser = false;
    bool dbgSymtab = false;
    bool dbgCodegen = false;
    std::string filename;

    if (argc < 2) {
        std::cout << "Error: Missing positional argument filename"\
        << std::endl;
        return 1;

    } else if (argc <= 6) {
        for (int i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "-h") ||
                !strcmp(argv[i], "--help")) {
                std::cout << "Usage: compiler file [options]" << std::endl;
                std::cout << "Options:" << std::endl;
                std::cout << "  -h | --help        Show help" << std::endl;
                std::cout << "  -d                 Show debug for all components" << std::endl;
                std::cout << "  --debug-scanner    Show debug for scanner" << std::endl;
                std::cout << "  --debug-parser     Show debug for parser" << std::endl;
                std::cout << "  --debug-symtab     Show symbol table on each scope exit" << std::endl;
                std::cout << "  --debug-codegen    Output LLVM IR in out.ll file" << std::endl;
                return 0;

            } else if (!strcmp(argv[i], "-d")) {
                // Debug all components
                dbgScanner = true;
                dbgParser = true;
                dbgSymtab = true;
                dbgCodegen = true;

            } else if (!strcmp(argv[i], "--debug-scanner")) {
                dbgScanner = true;

            } else if (!strcmp(argv[i], "--debug-parser")) {
                dbgParser = true;

            } else if (!strcmp(argv[i], "--debug-symtab")) {
                dbgSymtab = true;

            } else if (!strcmp(argv[i], "--debug-codegen")) {
                dbgCodegen = true;

            }
        }
    } else {
        std::cout << "Error: Too many arguments" << std::endl;
        return 1;
    }

    ScopeManager scoper(dbgSymtab);
    Scanner scanner(&scoper, dbgScanner);
    Parser parser(&scanner, &scoper, dbgParser, dbgCodegen);

    if (!scanner.openFile(argv[1])) {
        std::cout << "Error: File couldn't be opened" << std::endl;
        return 1;
    }

    bool success = parser.parse() && parser.outputAssembly();

    return !success;
}