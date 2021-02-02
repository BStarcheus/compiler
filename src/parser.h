#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

class Parser {
    public:
        Parser(Scanner* scannerPtr, bool dbg = false);
        ~Parser();
        bool debugFlag;
        bool errorFlag;
        bool parse();
    private:
        Token token;
        Scanner* scanner;

        void error(std::string msg);
        void warning(std::string msg);
        void debug(std::string msg);

        bool isTokenType(TokenType t);
        bool program();
        bool programHeader();
        bool programBody();
        bool declaration();

        bool procedureDeclaration();
        bool procedureHeader();
        bool parameterList();
        bool parameter();
        bool procedureBody();

        bool variableDeclaration();
        bool typeDeclaration();
        bool typeMark();
        bool bound();

        bool statement();
        bool procedureCall();
        bool assignmentStatement();
        bool destination();
        bool ifStatement();
        bool loopStatement();
        bool returnStatement();

        bool identifier();
        bool expression();
        bool expression_p();
        bool arithOp();
        bool arithOp_p();
        bool relation();
        bool relation_p();
        bool term();
        bool term_p();
        bool factor();
        bool procCallOrName();
        bool name();
        bool argumentList();
        bool number();
        bool string();

        bool declarationBlockHelper();
        bool statementBlockHelper();
};
#endif