#ifndef SCANNER_H
#define SCANNER_H

#include "tokenType.h"
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
        Scanner(bool dbg);
        ~Scanner();
        bool debugFlag;
        bool errorFlag;
        bool openFile(std::string filename);
        Token scan();
    private:
        std::ifstream file;
        std::string filename;
        int lineNum;
        std::unordered_map<char, CharClass> charClass;
        int getCharClass(char c);
        Token scanToken();
        void error(std::string msg);
        void warning(std::string msg);
        void debug(std::string msg);
        void debug(Token t);
};
#endif