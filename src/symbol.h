#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>
#include "token.h"

/* Symbols stored in the symbol table 
 * are either identifiers or reserved words
 */
class Symbol {
    public:
        Symbol();
        Symbol(std::string id_p, TokenType type_p);

        // Getters and setters
        std::string getId();
        void setId(std::string id_p);
        TokenType getType();
        void setType(TokenType type_p);
        bool isArray();
        void setIsArray(bool val);
        int getArrayBound();
        void setArrayBound(int bound);
    private:
        std::string id;
        TokenType type;
        bool isArr;
        int arrBound;
};
#endif