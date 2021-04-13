#ifndef SCOPE_H
#define SCOPE_H

#include "symbol.h"
#include <unordered_map>

typedef std::unordered_map<std::string, Symbol> SymbolTable;

class Scope {
    public:
        Scope();
        void setSymbol(std::string s, Symbol sym);
        Symbol getSymbol(std::string s);
        bool hasSymbol(std::string s);
        SymbolTable::iterator begin();
        SymbolTable::iterator end();
        void printSymbolTable();
        Scope* prev;
    private:
        SymbolTable table;
};
#endif