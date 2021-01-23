#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include "macro.h"
#include <string>
#include <fstream>

class Scanner {
    public:
        Scanner();
        ~Scanner();
        bool openFile(std::string filename);
        Token scan();
    private:
        std::ifstream file;
        int lineNum;
        int scanToken();
};
#endif