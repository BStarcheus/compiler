#ifndef SCANNER_H
#define SCANNER_H

#include "scopeManager.h"
#include "token.h"
#include <fstream>
#include <unordered_map>

enum CharClass { 
    NUM,
    LC_ALPHA,
    UC_ALPHA,
    SPACE,
    SPECIAL,
    INVALID = -2
};

class Scanner {
    public:
        Scanner(ScopeManager* scoperPtr, bool dbg);
        ~Scanner();
        bool debugFlag;
        bool errorFlag;
        bool openFile(std::string filename);
        Token scan();

        void error(std::string msg);
        void warning(std::string msg);
        void debug(std::string msg);
        void debug(Token t);
    private:
        ScopeManager* scoper;
        std::ifstream file;
        std::string filename;
        int lineNum;
        std::unordered_map<char, CharClass> charClass;
        int getCharClass(char c);
        Token scanToken();
};
#endif