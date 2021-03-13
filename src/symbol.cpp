#include "symbol.h"

Symbol::Symbol() {
    id = "";
    type = T_UNK;
    isArr = false;
    arrBound = 0;
}

Symbol::Symbol(std::string id_p, TokenType type_p) {
    id = id_p;
    type = type_p;
    isArr = false;
    arrBound = 0;
}


// Getters and setters

std::string Symbol::getId() { return id; }

void Symbol::setId(std::string id_p) { id = id_p; }

TokenType Symbol::getType() { return type; }

void Symbol::setType(TokenType type_p) { type = type_p; }

bool Symbol::isArray() { return isArr; }

void Symbol::setIsArray(bool val) { isArr = val; }

int Symbol::getArrayBound() { return arrBound; }

void Symbol::setArrayBound(int bound) { arrBound = bound; }
