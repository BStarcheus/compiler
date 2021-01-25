#ifndef TOKEN_H
#define TOKEN_H

struct Token {
    int type;
    union {
        char strVal[256];
        int intVal;
        float floatVal;
    } val;
};
#endif