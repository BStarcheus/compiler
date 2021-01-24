#ifndef TOKEN_H
#define TOKEN_H

struct Token {
    int type;
    union {
        char strVal[256];
        int intVal;
        double dblVal;
    } val;
};
#endif