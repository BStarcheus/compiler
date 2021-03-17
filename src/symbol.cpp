#include "symbol.h"

Symbol::Symbol() {
    id = "";
    tokenType = T_UNK;
    symbolType = ST_UNK;
    type = TYPE_UNK;
    isGlobal = false;
    isArr = false;
    arrSize = 0;
}

Symbol::Symbol(std::string id_p, TokenType tok_p) {
    id = id_p;
    tokenType = tok_p;
    symbolType = ST_UNK;
    type = TYPE_UNK;
    isGlobal = false;
    isArr = false;
    arrSize = 0;
}

Symbol::Symbol(std::string id_p, TokenType tok_p, SymbolType sym_p, Type type_p) {
    id = id_p;
    tokenType = tok_p;
    symbolType = sym_p;
    type = type_p;
    isGlobal = false;
    isArr = false;
    arrSize = 0;
}