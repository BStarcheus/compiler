#include "symbolTable.h"
#include "tokenType.h"

SymbolTable::SymbolTable() {
    // Add reserved words
    symbolTable["program"] = (Token){T_PROGRAM, "program"};
    symbolTable["is"] = (Token){T_IS, "is"};
    symbolTable["begin"] = (Token){T_BEGIN, "begin"};
    symbolTable["end"] = (Token){T_END, "end"};
    symbolTable["global"] = (Token){T_GLOBAL, "global"};
    symbolTable["procedure"] = (Token){T_PROCEDURE, "procedure"};
    symbolTable["variable"] = (Token){T_VARIABLE, "variable"};
    symbolTable["type"] = (Token){T_TYPE, "type"};
    symbolTable["integer"] = (Token){T_INTEGER, "integer"};
    symbolTable["float"] = (Token){T_FLOAT, "float"};
    symbolTable["string"] = (Token){T_STRING, "string"};
    symbolTable["bool"] = (Token){T_BOOL, "bool"};
    symbolTable["enum"] = (Token){T_ENUM, "enum"};
    symbolTable["if"] = (Token){T_IF, "if"};
    symbolTable["then"] = (Token){T_THEN, "then"};
    symbolTable["else"] = (Token){T_ELSE, "else"};
    symbolTable["for"] = (Token){T_FOR, "for"};
    symbolTable["return"] = (Token){T_RETURN, "return"};
    symbolTable["not"] = (Token){T_NOT, "not"};
    symbolTable["true"] = (Token){T_TRUE, "true"};
    symbolTable["false"] = (Token){T_FALSE, "false"};
}

void SymbolTable::setSymbol(std::string s, Token t) {
    symbolTable[s] = t;
}

Token SymbolTable::getSymbol(std::string s) {
    return symbolTable[s];
}