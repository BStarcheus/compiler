#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

class Parser {
    public:
        Parser(Scanner* scannerPtr, ScopeManager* scoperPtr, bool dbg);
        ~Parser();
        bool debugFlag;
        bool errorFlag;
        bool parse();
    private:
        Token token;
        Scanner* scanner;
        ScopeManager* scoper;

        void error(std::string msg);
        void warning(std::string msg);
        void debug(std::string msg);

        bool isTokenType(TokenType t);
        bool program();
        bool programHeader();
        bool programBody();
        bool declaration();

        bool procedureDeclaration(Symbol &decl);
        bool procedureHeader(Symbol &decl);
        bool parameterList(Symbol &decl);
        bool parameter(Symbol &param);
        bool procedureBody();

        bool variableDeclaration(Symbol &decl);
        bool typeMark(Symbol &id);
        bool bound(Symbol &id);

        bool statement();
        bool procedureCall(Symbol &id);
        bool assignmentStatement();
        bool destination(Symbol &id);
        bool ifStatement();
        bool loopStatement();
        bool returnStatement();

        bool identifier(Symbol &id);
        bool expression();
        bool expression_p();
        bool arithOp();
        bool arithOp_p();
        bool relation();
        bool relation_p();
        bool term();
        bool term_p();
        bool factor();
        bool procCallOrName(Symbol &id);
        bool name(Symbol &id);
        bool argumentList();
        bool number(Symbol &id);
        bool string();

        bool declarationBlockHelper();
        bool statementBlockHelper();
};
#endif