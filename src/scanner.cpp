#include "scanner.h"
#include <iostream>

Scanner::Scanner() {}

Scanner::~Scanner() {
    file.close();
}

bool Scanner::openFile(std::string filename) {
    file.open(filename);
    if (file.is_open()) {
        lineNum = 1;
        return true;
    }
    return false;
}

Token Scanner::scan() {
    Token token = Token();
    token.type = scanToken();

    // Handle adding to symbol table

    return token;
}

int Scanner::scanToken() {
    if (!file.is_open()) {
        // REPORT ERROR
        return T_EOF;
    }

    // Grab whitespace

    
    while (!file.eof()) {
        std::cout << (char)file.get() << std::endl;
    }
    return T_EOF;
}