#include "symbol.h"

Symbol::Symbol() {
    id = "";
    tokenType = T_UNK;
    symbolType = ST_UNK;
    type = TYPE_UNK;
    isGlobal = false;
    isArr = false;
    arrSize = 0;
    isIndexed = false;
    llvm_value = nullptr;
    llvm_function = nullptr;
    llvm_address = nullptr;
}

Symbol::Symbol(std::string id_p, TokenType tok_p) {
    id = id_p;
    tokenType = tok_p;
    symbolType = ST_UNK;
    type = TYPE_UNK;
    isGlobal = false;
    isArr = false;
    arrSize = 0;
    isIndexed = false;
    llvm_value = nullptr;
    llvm_function = nullptr;
    llvm_address = nullptr;
}

Symbol::Symbol(std::string id_p, TokenType tok_p, SymbolType sym_p, Type type_p) {
    id = id_p;
    tokenType = tok_p;
    symbolType = sym_p;
    type = type_p;
    isGlobal = false;
    isArr = false;
    arrSize = 0;
    isIndexed = false;
    llvm_value = nullptr;
    llvm_function = nullptr;
    llvm_address = nullptr;
}

// Get type name from type number
std::string getTypeName(Type t) {
    switch (t) {
        case TYPE_INT: return "integer";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_BOOL: return "bool";
        default: return "unknown";
    }
}