#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "token.h"
#include <string>
#include <unordered_map>

typedef std::unordered_map<std::string, Token> SymTabMap;

class SymbolTable {
    public:
        SymbolTable();
        void setSymbol(std::string s, Token t);
        Token getSymbol(std::string s);
        bool hasSybmol(std::string s);
        SymTabMap::iterator begin();
        SymTabMap::iterator end();
    private:
        SymTabMap symbolTable;
};

extern SymbolTable symb;

#endif