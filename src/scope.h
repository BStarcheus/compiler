#ifndef SCOPE_H
#define SCOPE_H

#include "token.h"
#include <string>
#include <unordered_map>

typedef std::unordered_map<std::string, Token> SymbolTable;

class Scope {
    public:
        Scope();
        ~Scope();
        void setSymbol(std::string s, Token t);
        Token getSymbol(std::string s);
        bool hasSymbol(std::string s);
        void printSymbolTable();
        Scope* prev;
    private:
        SymbolTable table;
};
#endif