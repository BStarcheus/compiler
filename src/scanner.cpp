#include "scanner.h"
#include <iostream>
#include <iomanip>

Scanner::Scanner(bool dbg) {
    debugFlag = dbg;

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
    this->filename = filename;
    file.open(filename);
    if (file.is_open()) {
        lineNum = 1;
        errorFlag = false;
        return true;
    }
    errorFlag = true;
    return false;
}

void Scanner::error(std::string msg) {
    std::cout << filename << ":" << std::left << std::setw(4);
    std::cout << lineNum << "  Error: " << msg << std::endl;
}
void Scanner::warning(std::string msg) {
    std::cout << filename << ":" << std::left << std::setw(4);
    std::cout << lineNum << "  Warning: " << msg << std::endl;
}
void Scanner::debug(std::string msg) {
    if (debugFlag) {
        std::cout << msg << std::endl;
    }
}
void Scanner::debug(Token t) {
    if (debugFlag) {
        std::cout << "token: " << std::left;
        std::cout << std::setw(15) << getTokenTypeName(t);
        if (t.type == T_STRING_VAL || t.type == T_IDENTIFIER) {
            std::cout << t.val.strVal;
        } else if (t.type == T_INTEGER_VAL) {
            std::cout << t.val.intVal;
        } else if (t.type == T_FLOAT_VAL) {
            std::cout << t.val.floatVal;
        }
        std::cout << std::endl;
    }
}

Token Scanner::scan() {
    Token token = scanToken();
    debug(token);

    // Handle adding to symbol table

    return token;
}

int Scanner::getCharClass(char c) {
    if (c == -1) {
        return -1;
    } else if (charClass.find(c) == charClass.end()) {
        // Invalid character
        return -2;
    }
    return charClass[c];
}

Token Scanner::scanToken() {
    Token token = Token();

    if (!file.is_open()) {
        errorFlag = true;
        error("File is not open");
        token.type = T_UNK;
        return token;
    }

    char ch, nextCh;
    int chClass;

    do {
        // Grab whitespace and invalid chars
        do {
            ch = file.get();
            if (ch == '\n') lineNum++;

            chClass = getCharClass(ch);
            if (chClass == -2) {
                errorFlag = true;
                error("Invalid character");
            }
        } while (chClass == SPACE || chClass == -2);

        // Grab comments
        if (ch == '/') {
            nextCh = file.get();

            if (nextCh == '/') {
                // Single line comment
                do {
                    nextCh = file.get();
                } while (nextCh != '\n' && nextCh != EOF);
                file.unget();

            } else if (nextCh == '*') {
                // Multiline comment
                // Allow nested comments

                int nestLayer = 1;
                while (nestLayer > 0) {
                    nextCh = file.get();

                    if (nextCh == '*') {
                        nextCh = file.get();
                        if (nextCh == '/') {
                            // Close
                            nestLayer--;
                        }
                    } else if (nextCh == '/') {
                        nextCh = file.get();
                        if (nextCh == '*') {
                            nestLayer++;
                        }
                    } 
                    
                    // Add newline if found in any of the above execution paths
                    if (nextCh == '\n') {
                        lineNum++;
                    }
                }
            } else {
                // Division handled below
                file.unget();
                break;
            }
        }
    // If you hit the if block above for comments, ch will always be / here,
    // so it will loop back to the whitespace block and get the next ch
    } while (chClass == SPACE || ch == '/');

    switch (chClass) {
        case SPECIAL:
            switch (ch) {
                // Single char tokens
                case '.': token.type = T_PERIOD; break;
                case ',': token.type = T_COMMA; break;
                case ';': token.type = T_SEMICOLON; break;
                case '(': token.type = T_LPAREN; break;
                case ')': token.type = T_RPAREN; break;
                case '[': token.type = T_LBRACKET; break;
                case ']': token.type = T_RBRACKET; break;
                case '{': token.type = T_LBRACE; break;
                case '}': token.type = T_RBRACE; break;
                case '+': token.type = T_PLUS; break;
                case '-': token.type = T_MINUS; break;
                case '*': token.type = T_MULTIPLY; break;
                case '/': token.type = T_DIVIDE; break;
                case '&': token.type = T_AND; break;
                case '|': token.type = T_OR; break;

                // Operators
                case '<':
                    ch = file.get();
                    if (ch == '=') {
                        token.type = T_LESS_EQ;
                    } else {
                        file.unget();
                        token.type = T_LESS;
                    }
                    break;
                case '>':
                    ch = file.get();
                    if (ch == '=') {
                        token.type = T_GREATER_EQ;
                    } else {
                        file.unget();
                        token.type = T_GREATER;
                    }
                    break;
                case '=':
                    ch = file.get();
                    if (ch == '=') {
                        token.type = T_EQUAL;
                    } else {
                        file.unget();
                        token.type = T_UNK;
                    }
                    break;
                case '!':
                    ch = file.get();
                    if (ch == '=') {
                        token.type = T_NOT_EQUAL;
                    } else {
                        file.unget();
                        token.type = T_UNK;
                    }
                    break;
                case ':':
                    ch = file.get();
                    if (ch == '=') {
                        token.type = T_ASSIGNMENT;
                    } else {
                        file.unget();
                        token.type = T_UNK;
                    }
                    break;

                // Strings
                case '\"': {
                    token.type = T_STRING_VAL;
                    int i = 0;

                    ch = file.get();
                    while (ch != '\"') {
                        if (ch == '\n' || ch == EOF) {
                            file.unget();
                            token.type = T_UNK;
                            // TODO Report error
                            break;
                        } else if (i >= 255) {
                            errorFlag = true;
                            error("String value too large for buffer");
                            break;
                        }

                        token.val.strVal[i++] = ch;
                        ch = file.get();
                    }
                    token.val.strVal[i] = '\0';
                    break;
                }

                default: token.type = T_UNK; break;
            }
            break;

        case NUM: {
            std::string str = "";
            do {
                if (ch != '_') str += ch;
                ch = file.get();
            } while (getCharClass(ch) == NUM || ch == '_');

            if (ch == '.') {
                // Float
                do {
                    if (ch != '_') str += ch;
                    ch = file.get();
                } while (getCharClass(ch) == NUM || ch == '_');
                
                token.type = T_FLOAT_VAL;
                token.val.floatVal = std::stof(str);

                // Put back char from next token
                file.unget();

            } else {
                // Integer
                token.type = T_INTEGER_VAL;
                token.val.intVal = std::stoi(str);

                // Put back char from next token
                file.unget();
            }
            break;
        }
        case LC_ALPHA:
        case UC_ALPHA:
            token.type = T_IDENTIFIER;
            break;

        default:
            if (ch == EOF) {
                token.type = T_EOF;
            } else {
                token.type = T_UNK;
            }
            break;
    }
    return token;
}