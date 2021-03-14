#include "symbol.h"

Symbol::Symbol() {
    id = "";
    tokenType = T_UNK;
    symbolType = ST_UNK;
    type = TYPE_UNK;
    isArr = false;
    arrBound = 0;
}

Symbol::Symbol(std::string id_p, TokenType tok_p) {
    id = id_p;
    tokenType = tok_p;
    symbolType = ST_UNK;
    type = TYPE_UNK;
    isArr = false;
    arrBound = 0;
}

Symbol::Symbol(std::string id_p, TokenType tok_p, SymbolType sym_p, Type type_p) {
    id = id_p;
    tokenType = tok_p;
    symbolType = sym_p;
    type = type_p;
    isArr = false;
    arrBound = 0;
}