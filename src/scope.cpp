#include "scope.h"
#include <iostream>

Scope::Scope() {
    prev = nullptr;
}

Scope::~Scope() {
    
}

void Scope::setSymbol(std::string s, Token t) {
    table[s] = t;
}

Token Scope::getSymbol(std::string s) {
    if (hasSymbol(s)) {
        return table[s];
    } else {
        Token t = {T_UNK, ""};
        return t;
    }
}

bool Scope::hasSymbol(std::string s) {
    return table.find(s) != table.end();
}

void Scope::printSymbolTable() {
    std::cout << std::endl;
    for (SymbolTable::iterator it = table.begin(); it != table.end(); ++it) {
        std::cout << getTokenTypeName(it->second.type) << " "\
            << it->second.val << std::endl;
    }
    std::cout << std::endl;
}