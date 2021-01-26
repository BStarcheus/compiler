#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "token.h"
#include <string>
#include <unordered_map>

// Placeholder
class SymbolTable {
    public:
        SymbolTable();
        void setSymbol(std::string s, Token t);
        Token getSymbol(std::string s);
    private:
        std::unordered_map<std::string, Token> symbolTable;
};

extern SymbolTable symb;

#endif