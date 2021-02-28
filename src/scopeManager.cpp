#include "scopeManager.h"
#include "tokenType.h"

ScopeManager::ScopeManager(bool dbg) {
    debugFlag = dbg;
    global = new Scope();
    local = nullptr;

    // Initialize global table with reserved words
    global->setSymbol("program", (Token){T_PROGRAM, "program"});
    global->setSymbol("is", (Token){T_IS, "is"});
    global->setSymbol("begin", (Token){T_BEGIN, "begin"});
    global->setSymbol("end", (Token){T_END, "end"});
    global->setSymbol("global", (Token){T_GLOBAL, "global"});
    global->setSymbol("procedure", (Token){T_PROCEDURE, "procedure"});
    global->setSymbol("variable", (Token){T_VARIABLE, "variable"});
    global->setSymbol("type", (Token){T_TYPE, "type"});
    global->setSymbol("integer", (Token){T_INTEGER, "integer"});
    global->setSymbol("float", (Token){T_FLOAT, "float"});
    global->setSymbol("string", (Token){T_STRING, "string"});
    global->setSymbol("bool", (Token){T_BOOL, "bool"});
    global->setSymbol("enum", (Token){T_ENUM, "enum"});
    global->setSymbol("if", (Token){T_IF, "if"});
    global->setSymbol("then", (Token){T_THEN, "then"});
    global->setSymbol("else", (Token){T_ELSE, "else"});
    global->setSymbol("for", (Token){T_FOR, "for"});
    global->setSymbol("return", (Token){T_RETURN, "return"});
    global->setSymbol("not", (Token){T_NOT, "not"});
    global->setSymbol("true", (Token){T_TRUE, "true"});
    global->setSymbol("false", (Token){T_FALSE, "false"});
}

ScopeManager::~ScopeManager() {
    delete global;
    delete local;
}

void ScopeManager::newScope() {
    Scope* temp = new Scope();
    temp->prev = local;
    local = temp;
}

void ScopeManager::exitScope() {
    if (debugFlag) {
        printScope(false);
    }

    if (local != nullptr) {
        Scope* temp = local;
        local = local->prev;
        delete temp;
    }
}

void ScopeManager::setSymbol(std::string s, Token t, bool g) {
    Scope* ptr = local;
    if (g) { ptr = global; }
    
    if (ptr != nullptr) {
        if (!ptr->hasSymbol(s)) {
            ptr->setSymbol(s, t);
        }
    }
}

// Set symbol for procedure at procedure's scope and one above
void ScopeManager::setProcSymbol(std::string s, Token t, bool g) {
    // Add local within procedure, or globally
    setSymbol(s, t, g);

    // If local, add to one scope above
    if (!g) {
        Scope* ptr = local->prev;
        if (ptr != nullptr) {
            if (!ptr->hasSymbol(s)) {
                ptr->setSymbol(s, t);
            }
        }
    }
}

// Get symbol, or return unknown token if no symbol
Token ScopeManager::getSymbol(std::string s, bool g) {
    if (g) {
        if (global->hasSymbol(s)) {
            return global->getSymbol(s);
        }
    } else {
        if (local->hasSymbol(s)) {
            return local->getSymbol(s);
        }
    }
    Token t = {T_UNK, ""};
    return t;
}

bool ScopeManager::hasSymbol(std::string s, bool g) {
    if (g) {
        return global->hasSymbol(s);
    } else {
        return local->hasSymbol(s);
    }
}

void ScopeManager::printScope(bool g) {
    if (g) {
        global->printSymbolTable();
    } else {
        local->printSymbolTable();
    }
}