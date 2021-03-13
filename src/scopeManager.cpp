#include "scopeManager.h"

ScopeManager::ScopeManager(bool dbg) {
    debugFlag = dbg;
    errorFlag = false;
    global = new Scope();
    local = global;

    // Initialize global table with reserved words
    global->setSymbol("program", Symbol("program", T_PROGRAM));
    global->setSymbol("is", Symbol("is", T_IS));
    global->setSymbol("begin", Symbol("begin", T_BEGIN));
    global->setSymbol("end", Symbol("end", T_END));
    global->setSymbol("global", Symbol("global", T_GLOBAL));
    global->setSymbol("procedure", Symbol("procedure", T_PROCEDURE));
    global->setSymbol("variable", Symbol("variable", T_VARIABLE));
    global->setSymbol("integer", Symbol("integer", T_INTEGER));
    global->setSymbol("float", Symbol("float", T_FLOAT));
    global->setSymbol("string", Symbol("string", T_STRING));
    global->setSymbol("bool", Symbol("bool", T_BOOL));
    global->setSymbol("if", Symbol("if", T_IF));
    global->setSymbol("then", Symbol("then", T_THEN));
    global->setSymbol("else", Symbol("else", T_ELSE));
    global->setSymbol("for", Symbol("for", T_FOR));
    global->setSymbol("return", Symbol("return", T_RETURN));
    global->setSymbol("not", Symbol("not", T_NOT));
    global->setSymbol("true", Symbol("true", T_TRUE));
    global->setSymbol("false", Symbol("false", T_FALSE));
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

// Set symbol for procedure at procedure's scope and one above
void ScopeManager::setProcSymbol(std::string s, Symbol sym, bool g) {
    // Add local within procedure, or globally
    setSymbol(s, sym, g);

    // If not global scope, add to previous scope
    if (!g) {
        Scope* ptr = local->prev;
        if (ptr != nullptr) {
            if (!ptr->hasSymbol(s)) {
                ptr->setSymbol(s, sym);
            }
        }
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