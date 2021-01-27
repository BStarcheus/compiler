#ifndef TOKEN_H
#define TOKEN_H

#include <string>

struct Token {
    int type;
    std::string val;
    int getIntVal() { return std::stoi(val); }
    float getFloatVal() { return std::stof(val); }
};
#endif