#ifndef SCOPEMANAGER_H
#define SCOPEMANAGER_H

#include "scope.h"

class ScopeManager {
    public:
        ScopeManager(bool dbg);
        ~ScopeManager();
        void newScope();
        void exitScope();
        void setSymbol(std::string s, Symbol sym, bool g);
        Symbol getSymbol(std::string s);
        Symbol getSymbol(std::string s, bool g);
        bool hasSymbol(std::string s);
        bool hasSymbol(std::string s, bool g);
        SymbolTable::iterator getScopeBegin(bool g = false);
        SymbolTable::iterator getScopeEnd(bool g = false);
        bool isCurrentScopeGlobal();

        void setCurrentProcedure(Symbol proc);
        Symbol getCurrentProcedure();

        void printScope(bool g);
        bool debugFlag;
        bool errorFlag;
    private:
        Scope* global;
        Scope* local;

        // Used to find current procedure symbol in local scope
        std::string _CUR_PROC = "_CUR_PROC";
};
#endif