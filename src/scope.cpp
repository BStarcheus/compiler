#include "scope.h"
#include <iostream>

Scope::Scope() {
    prev = nullptr;
}

Scope::~Scope() {
    
}

void Scope::setSymbol(std::string s, Symbol sym) {
    table[s] = sym;
}

Symbol Scope::getSymbol(std::string s) {
    if (hasSymbol(s)) {
        return table[s];
    } else {
        return Symbol();
    }
}

bool Scope::hasSymbol(std::string s) {
    return table.find(s) != table.end();
}

void Scope::printSymbolTable() {
    std::cout << std::endl;
    for (SymbolTable::iterator it = table.begin(); it != table.end(); ++it) {
        std::cout << getTokenTypeName(it->second.tokenType) << " "\
            << it->second.id << std::endl;
    }
    std::cout << std::endl;
}