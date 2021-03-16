#include "scopeManager.h"

ScopeManager::ScopeManager(bool dbg) {
    debugFlag = dbg;
    errorFlag = false;
    global = new Scope();
    local = global;

    // Initialize global table with reserved words
    global->setSymbol("program", Symbol("program", T_PROGRAM, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("is", Symbol("is", T_IS, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("begin", Symbol("begin", T_BEGIN, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("end", Symbol("end", T_END, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("global", Symbol("global", T_GLOBAL, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("procedure", Symbol("procedure", T_PROCEDURE, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("variable", Symbol("variable", T_VARIABLE, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("integer", Symbol("integer", T_INTEGER, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("float", Symbol("float", T_FLOAT, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("string", Symbol("string", T_STRING, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("bool", Symbol("bool", T_BOOL, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("if", Symbol("if", T_IF, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("then", Symbol("then", T_THEN, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("else", Symbol("else", T_ELSE, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("for", Symbol("for", T_FOR, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("return", Symbol("return", T_RETURN, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("not", Symbol("not", T_NOT, ST_KEYWORD, TYPE_UNK));
    global->setSymbol("true", Symbol("true", T_TRUE, ST_KEYWORD, TYPE_BOOL));
    global->setSymbol("false", Symbol("false", T_FALSE, ST_KEYWORD, TYPE_BOOL));
}

ScopeManager::~ScopeManager() {
    if (global != local) {
        delete global;
    }
    delete local;
}

// Enter a new local scope
void ScopeManager::newScope() {
    Scope* temp = new Scope();
    temp->prev = local;
    local = temp;
}

// Exit a local scope. Set local scope to previous scope.
// If trying to exit outermost/global scope, do nothing.
void ScopeManager::exitScope() {
    if (debugFlag) {
        printScope(false);
    }

    if (local != nullptr && local != global) {
        Scope* temp = local;
        local = local->prev;
        delete temp;
    }
}

void ScopeManager::setSymbol(std::string s, Symbol sym, bool g) {
    if (g) {
        global->setSymbol(s, sym);
    } else {
        local->setSymbol(s, sym);
    }
}

// First check local, then global scope
Symbol ScopeManager::getSymbol(std::string s) {
    if (local->hasSymbol(s)) {
        return local->getSymbol(s);
    } else {
        // If not in global, UNK
        return global->getSymbol(s);
    }
}

// Get symbol, or return unknown token if no symbol
Symbol ScopeManager::getSymbol(std::string s, bool g) {
    if (g) {
        return global->getSymbol(s);
    } else {
        return local->getSymbol(s);
    }
}

// First check local, then global scope
bool ScopeManager::hasSymbol(std::string s) {
    return local->hasSymbol(s) || global->hasSymbol(s);
}

// Check specific scope
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