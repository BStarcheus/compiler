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

SymbolTable::iterator Scope::begin() {
    return table.begin();
}

SymbolTable::iterator Scope::end() {
    return table.end();
}

void Scope::printSymbolTable() {
    std::cout << std::endl << "Scope Symbols:" << std::endl;
    for (auto const &sym: table) {
        std::cout << getTokenTypeName(sym.second.tokenType) << " "\
            << sym.second.id << std::endl;
    }
    std::cout << std::endl;
}