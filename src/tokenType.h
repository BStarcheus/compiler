#ifndef TOKENTYPE_H
#define TOKENTYPE_H

enum TokenType {
    // Reserved words
    T_PROGRAM = 257,
    T_IS = 258,
    T_BEGIN = 259,
    T_END = 260,
    T_GLOBAL = 261,
    T_PROCEDURE = 262,
    T_VARIABLE = 263,
    T_TYPE = 264,
    T_INTEGER = 265,
    T_FLOAT = 266,
    T_STRING = 267,
    T_BOOL = 268,
    T_ENUM = 269,
    T_IF = 270,
    T_THEN = 271,
    T_ELSE = 272,
    T_FOR = 273,
    T_RETURN = 274,
    T_NOT = 275,
    T_TRUE = 276,
    T_FALSE = 277,

    // Single char tokens
    T_PERIOD = '.',
    T_SEMICOLON = ';',
    T_LPAREN = '(',
    T_RPAREN = ')',
    T_COMMA = ',',
    T_LBRACKET = '[',
    T_RBRACKET = ']',
    T_LBRACE = '{',
    T_RBRACE = '}',
    T_PLUS = '+',
    T_MINUS = '-',
    T_ASTERISK = '*',
    T_FSLASH = '/',
    T_LESS = '<',
    T_GREATER = '>',
    
    // Operators
    T_LESS_EQ = 280,
    T_GREATER_EQ = 281,
    T_EQUAL = 282,
    T_NOT_EQUAL = 283,
    T_AND = 284,
    T_OR = 285,
    T_ASSIGNMENT = 286,

    T_IDENTIFIER = 300,

    // Constants
    T_NUMBER_VAL = 305,
    T_STRING_VAL = 306,

    T_EOF = 349,
    T_UNK = 350
};
#endif