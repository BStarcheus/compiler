#include "symbol.h"

Symbol::Symbol() {
    id = "";
    tokType = T_UNK;
    symType = ST_UNK;
    type = TYPE_UNK;
    isArr = false;
    arrBound = 0;
}

Symbol::Symbol(std::string id_p, TokenType tok_p) {
    id = id_p;
    tokType = tok_p;
    symType = ST_UNK;
    type = TYPE_UNK;
    isArr = false;
    arrBound = 0;
}

Symbol::Symbol(std::string id_p, TokenType tok_p, SymbolType sym_p, Type type_p) {
    id = id_p;
    tokType = tok_p;
    symType = sym_p;
    type = type_p;
    isArr = false;
    arrBound = 0;
}

// Getters and setters

std::string Symbol::getId() { return id; }

void Symbol::setId(std::string id_p) { id = id_p; }

TokenType Symbol::getTokenType() { return tokType; }

void Symbol::setTokenType(TokenType tok_p) { tokType = tok_p; }

SymbolType Symbol::getSymbolType() { return symType; }

void Symbol::setSymbolType(SymbolType sym_p) { symType = sym_p; }

Type Symbol::getType() { return type; }

void Symbol::setType(Type type_p) { type = type_p; }

bool Symbol::isArray() { return isArr; }

void Symbol::setIsArray(bool val) { isArr = val; }

int Symbol::getArrayBound() { return arrBound; }

void Symbol::setArrayBound(int bound) { arrBound = bound; }