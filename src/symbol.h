#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>
#include "token.h"


/* Type of the variable, or
 * return type of the procedure
 */
enum Type {
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_UNK
};

/* Symbols are either reserved words or identifiers.
 * Identifiers are either variables or procedures.
 * 
 * ST_UNK for intermediate symbols for type checking
 */
enum SymbolType {
    ST_KEYWORD,
    ST_VARIABLE,
    ST_PROCEDURE,
    ST_UNK
};


/* Symbols stored in the symbol table 
 * are either identifiers or reserved words
 * 
 * Also used to represent expressions, etc for type checking
 */
struct Symbol {
    Symbol();
    Symbol(std::string id_p, TokenType tok_p);
    Symbol(std::string id_p, TokenType tok_p, SymbolType sym_p, Type type_p);

    // Identifier value
    std::string id;

    // Token type is either identifier or the reserved word value
    TokenType tokenType;

    // Symbol type is reserved keyword, variable, or procdure
    SymbolType symbolType;

    // Type of variable, or return type of procedure
    Type type;

    // Is it an array, and if so what is the bound
    bool isArr;
    int arrBound;
};
#endif