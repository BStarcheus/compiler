#include "tokenType.h"

// Get token name from type number
const char* getTokenTypeName(Token t) {
    static bool isInit;
    static const char* names[351];
    if (!isInit) {
#define INSERT_ELEMENT(p) names[p] = #p
        INSERT_ELEMENT(T_PROGRAM);
        INSERT_ELEMENT(T_IS);
        INSERT_ELEMENT(T_BEGIN);
        INSERT_ELEMENT(T_END);
        INSERT_ELEMENT(T_GLOBAL);
        INSERT_ELEMENT(T_PROCEDURE);
        INSERT_ELEMENT(T_VARIABLE);
        INSERT_ELEMENT(T_TYPE);
        INSERT_ELEMENT(T_INTEGER);
        INSERT_ELEMENT(T_FLOAT);
        INSERT_ELEMENT(T_STRING);
        INSERT_ELEMENT(T_BOOL);
        INSERT_ELEMENT(T_ENUM);
        INSERT_ELEMENT(T_IF);
        INSERT_ELEMENT(T_THEN);
        INSERT_ELEMENT(T_ELSE);
        INSERT_ELEMENT(T_FOR);
        INSERT_ELEMENT(T_RETURN);
        INSERT_ELEMENT(T_NOT);
        INSERT_ELEMENT(T_TRUE);
        INSERT_ELEMENT(T_FALSE);

        INSERT_ELEMENT(T_PERIOD);
        INSERT_ELEMENT(T_SEMICOLON);
        INSERT_ELEMENT(T_COLON);
        INSERT_ELEMENT(T_LPAREN);
        INSERT_ELEMENT(T_RPAREN);
        INSERT_ELEMENT(T_COMMA);
        INSERT_ELEMENT(T_LBRACKET);
        INSERT_ELEMENT(T_RBRACKET);
        INSERT_ELEMENT(T_LBRACE);
        INSERT_ELEMENT(T_RBRACE);
        INSERT_ELEMENT(T_PLUS);
        INSERT_ELEMENT(T_MINUS);
        INSERT_ELEMENT(T_MULTIPLY);
        INSERT_ELEMENT(T_DIVIDE);
        INSERT_ELEMENT(T_LESS);
        INSERT_ELEMENT(T_GREATER);
        INSERT_ELEMENT(T_AND);
        INSERT_ELEMENT(T_OR);

        INSERT_ELEMENT(T_LESS_EQ);
        INSERT_ELEMENT(T_GREATER_EQ);
        INSERT_ELEMENT(T_EQUAL);
        INSERT_ELEMENT(T_NOT_EQUAL);
        INSERT_ELEMENT(T_ASSIGNMENT);

        INSERT_ELEMENT(T_IDENTIFIER);

        INSERT_ELEMENT(T_INTEGER_VAL);
        INSERT_ELEMENT(T_FLOAT_VAL);
        INSERT_ELEMENT(T_STRING_VAL);

        INSERT_ELEMENT(T_EOF);
        INSERT_ELEMENT(T_UNK);
#undef INSERT_ELEMENT
        isInit = true;
    }   

    return names[t.type];
}
