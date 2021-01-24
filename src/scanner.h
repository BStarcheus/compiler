#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include "tokenType.h"
#include <string>
#include <fstream>
#include <unordered_map>

enum CharClass { 
    NUM,
    LC_ALPHA,
    UC_ALPHA,
    SPACE,
    SPECIAL
};

class Scanner {
    public:
        Scanner();
        ~Scanner();
        bool openFile(std::string filename);
        Token scan();
    private:
        std::ifstream file;
        int lineNum;
        std::unordered_map<char, CharClass> charClass;
        int getCharClass(char c);
        int scanToken();
};
#endif