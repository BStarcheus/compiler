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

        bool procedureDeclaration(bool &isGlobal);
        bool procedureHeader(bool &isGlobal);
        bool parameterList();
        bool parameter();
        bool procedureBody();

        bool variableDeclaration(bool &isGlobal);
        bool typeDeclaration(bool &isGlobal);
        bool typeDef();
        bool typeMark();
        bool bound();

        bool statement();
        bool procedureCall(Token &id);
        bool assignmentStatement();
        bool destination(Token &id);
        bool ifStatement();
        bool loopStatement();
        bool returnStatement();

        bool identifier(Token &id);
        bool expression();
        bool expression_p();
        bool arithOp();
        bool arithOp_p();
        bool relation();
        bool relation_p();
        bool term();
        bool term_p();
        bool factor();
        bool procCallOrName(Token &id);
        bool name(Token &id);
        bool argumentList();
        bool number();
        bool string();

        bool declarationBlockHelper();
        bool statementBlockHelper();
};
#endif