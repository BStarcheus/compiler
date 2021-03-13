#ifndef SCOPEMANAGER_H
#define SCOPEMANAGER_H

#include "scope.h"

class ScopeManager {
    public:
        ScopeManager(bool dbg);
        ~ScopeManager();
        void newScope();
        void exitScope();
        void setSymbol(std::string s, Token t, bool g);
        void setProcSymbol(std::string s, Token t, bool g);
        Token getSymbol(std::string s, bool g);
        bool hasSymbol(std::string s, bool g);
        void printScope(bool g);
        bool debugFlag;
        bool errorFlag;
    private:
        Scope* global;
        Scope* local;
};
#endif