#include "scanner.h"
#include <iostream>
#include <iomanip>

Scanner::Scanner(ScopeManager* scoperPtr, bool dbg) {
    debugFlag = dbg;
    errorFlag = false;
    scoper = scoperPtr;

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
    errorFlag = true;
    std::cout << filename << ":" << std::left << std::setw(4);
    std::cout << lineNum << "  Error: " << msg << std::endl;
}
void Scanner::warning(std::string msg) {
    std::cout << filename << ":" << std::left << std::setw(4);
    std::cout << lineNum << "  Warning: " << msg << std::endl;
}
void Scanner::debug(std::string msg) {
    if (debugFlag) {
        std::cout << filename << ":" << std::left << std::setw(4);
        std::cout << lineNum << "  Debug: " << msg << std::endl;
    }
}
void Scanner::debug(Token t) {
    if (debugFlag) {
        std::cout << "token: " << std::left;
        std::cout << std::setw(15) << getTokenTypeName(t.type);
        std::cout << t.val << std::endl;
    }
}

Token Scanner::scan() {
    Token token = scanToken();
    debug(token);
    return token;
}

int Scanner::getCharClass(char c) {
    if (c == EOF) {
        return EOF;
    } else if (charClass.find(c) == charClass.end()) {
        // Invalid character
        return INVALID;
    }
    return charClass[c];
}

Token Scanner::scanToken() {
    Token token = Token();

    if (!file.is_open()) {
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
            chClass = getCharClass(ch);
            
            if (ch == '\n') {
                // LF
                lineNum++;
            } else if (ch == '\r') {
                // CRLF
                ch = file.get();
                if (ch == '\n') {
                    lineNum++;
                } else {
                    file.unget();
                    error("Invalid character");
                }
            } else if (chClass == INVALID) {
                error("Invalid character");
            }
        } while (chClass == SPACE || chClass == INVALID);

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
                    
                    if (nextCh == '\n') {
                        // Add newline if found in any of the above execution paths
                        lineNum++;
                    } else if (nextCh == EOF) {
                        // Valid to hit EOF with no close
                        // Return EOF token
                        break;
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
    } while (ch == '/');

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
                        token.type = T_COLON;
                    }
                    break;

                // Strings
                case '\"': {
                    token.type = T_STRING_VAL;

                    ch = file.get();
                    while (ch != '\"') {
                        if (ch == '\n') {
                            lineNum++;
                        } else if (ch == EOF) {
                            token.type = T_UNK;
                            error("String value missing closing quote");
                            break;
                        }
                        token.val += ch;
                        ch = file.get();
                    }
                    break;
                }

                default: token.type = T_UNK; break;
            }
            break;

        case NUM: {
            do {
                if (ch != '_') token.val += ch;
                ch = file.get();
                chClass = getCharClass(ch);
            } while (chClass == NUM || ch == '_');

            if (ch == '.') {
                // Float
                do {
                    if (ch != '_') token.val += ch;
                    ch = file.get();
                    chClass = getCharClass(ch);
                } while (chClass == NUM || ch == '_');
                
                token.type = T_FLOAT_VAL;

                // Put back char from next token
                file.unget();

            } else {
                // Integer
                token.type = T_INTEGER_VAL;

                // Put back char from next token
                file.unget();
            }
            break;
        }
        case LC_ALPHA:
        case UC_ALPHA:
            do {
                if (chClass == UC_ALPHA) {
                    // Type insensitive
                    // All keywords and id's stored lowercase
                    ch += ('a' - 'A');
                }
                token.val += ch;

                ch = file.get();
                chClass = getCharClass(ch);
            } while (chClass == LC_ALPHA ||
                     chClass == UC_ALPHA ||
                     chClass == NUM ||
                     ch == '_');

            // Put back char from next token
            file.unget();

            if (scoper->hasSymbol(token.val, true)) {
                // Check if reserved word, or id already in table
                token.type = scoper->getSymbol(token.val, true).tokenType;

                // Note: Only global table checked since all reserved words in global.
                // If in local table, it would be an id anyway.
            } else {
                token.type = T_IDENTIFIER;
            }
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