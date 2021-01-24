#include "scanner.h"
#include <iostream>

Scanner::Scanner() {
    // Init character class map
    // Defines all characters that could signal start of tokens
    // Note: Other characters are legal in strings, but token starts with quote
    charClass['0'] = NUM;
    charClass['1'] = NUM;
    charClass['2'] = NUM;
    charClass['3'] = NUM;
    charClass['4'] = NUM;
    charClass['5'] = NUM;
    charClass['6'] = NUM;
    charClass['7'] = NUM;
    charClass['8'] = NUM;
    charClass['9'] = NUM;
    charClass['a'] = LC_ALPHA;
    charClass['b'] = LC_ALPHA;
    charClass['c'] = LC_ALPHA;
    charClass['d'] = LC_ALPHA;
    charClass['e'] = LC_ALPHA;
    charClass['f'] = LC_ALPHA;
    charClass['g'] = LC_ALPHA;
    charClass['h'] = LC_ALPHA;
    charClass['i'] = LC_ALPHA;
    charClass['j'] = LC_ALPHA;
    charClass['k'] = LC_ALPHA;
    charClass['l'] = LC_ALPHA;
    charClass['m'] = LC_ALPHA;
    charClass['n'] = LC_ALPHA;
    charClass['o'] = LC_ALPHA;
    charClass['p'] = LC_ALPHA;
    charClass['q'] = LC_ALPHA;
    charClass['r'] = LC_ALPHA;
    charClass['s'] = LC_ALPHA;
    charClass['t'] = LC_ALPHA;
    charClass['u'] = LC_ALPHA;
    charClass['v'] = LC_ALPHA;
    charClass['w'] = LC_ALPHA;
    charClass['x'] = LC_ALPHA;
    charClass['y'] = LC_ALPHA;
    charClass['z'] = LC_ALPHA;
    charClass['A'] = UC_ALPHA;
    charClass['B'] = UC_ALPHA;
    charClass['C'] = UC_ALPHA;
    charClass['D'] = UC_ALPHA;
    charClass['E'] = UC_ALPHA;
    charClass['F'] = UC_ALPHA;
    charClass['G'] = UC_ALPHA;
    charClass['H'] = UC_ALPHA;
    charClass['I'] = UC_ALPHA;
    charClass['J'] = UC_ALPHA;
    charClass['K'] = UC_ALPHA;
    charClass['L'] = UC_ALPHA;
    charClass['M'] = UC_ALPHA;
    charClass['N'] = UC_ALPHA;
    charClass['O'] = UC_ALPHA;
    charClass['P'] = UC_ALPHA;
    charClass['Q'] = UC_ALPHA;
    charClass['R'] = UC_ALPHA;
    charClass['S'] = UC_ALPHA;
    charClass['T'] = UC_ALPHA;
    charClass['U'] = UC_ALPHA;
    charClass['V'] = UC_ALPHA;
    charClass['W'] = UC_ALPHA;
    charClass['X'] = UC_ALPHA;
    charClass['Y'] = UC_ALPHA;
    charClass['Z'] = UC_ALPHA;
    charClass[' '] = SPACE;
    charClass['\t'] = SPACE;
    charClass['\n'] = SPACE;
    charClass['!'] = SPECIAL;
    charClass['&'] = SPECIAL;
    charClass['*'] = SPECIAL;
    charClass['('] = SPECIAL;
    charClass[')'] = SPECIAL;
    charClass['-'] = SPECIAL;
    charClass['='] = SPECIAL;
    charClass['+'] = SPECIAL;
    charClass['['] = SPECIAL;
    charClass[']'] = SPECIAL;
    charClass['{'] = SPECIAL;
    charClass['}'] = SPECIAL;
    charClass['|'] = SPECIAL;
    charClass[':'] = SPECIAL;
    charClass[';'] = SPECIAL;
    charClass['\"'] = SPECIAL;
    charClass['|'] = SPECIAL;
    charClass[','] = SPECIAL;
    charClass['.'] = SPECIAL;
    charClass['<'] = SPECIAL;
    charClass['>'] = SPECIAL;
    charClass['/'] = SPECIAL;
}

Scanner::~Scanner() {
    file.close();
}

bool Scanner::openFile(std::string filename) {
    file.open(filename);
    if (file.is_open()) {
        lineNum = 1;
        return true;
    }
    return false;
}

Token Scanner::scan() {
    Token token = Token();
    token.type = scanToken();

    // Handle adding to symbol table

    return token;
}

int Scanner::getCharClass(char c) {
    if (charClass.find(c) == charClass.end()) {
        // Invalid character to start a token
        // or EOF
        return -1;
    } else {
        return charClass[c];
    }
}

int Scanner::scanToken() {
    if (!file.is_open()) {
        // REPORT ERROR
        return T_EOF;
    }

    char ch = file.get();
    int chClass = getCharClass(ch);
    
    // Grab whitespace
    while (chClass == SPACE) {
        if (ch == '\n') lineNum++;
        ch = file.get();
        chClass = getCharClass(ch);
    }

    switch (chClass) {
        case SPECIAL:
            return T_PERIOD;

        case NUM:
            return T_NUMBER_VAL;

        case LC_ALPHA:
        case UC_ALPHA:
            return T_IDENTIFIER;

        default:
            if (ch == EOF) return T_EOF;

            return T_UNK;
    }
}