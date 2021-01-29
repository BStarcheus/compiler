#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

class Parser {
    public:
        Parser();
        ~Parser();
        bool parse();
    private:
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
        bool precedureCall();
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
        bool name();
        bool argumentList();
        bool number();
        bool string();
};
#endif