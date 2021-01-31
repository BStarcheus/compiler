#include "parser.h"
#include "tokenType.h"
#include <iostream>

Parser::Parser(Scanner* scannerPtr, bool dbg) {
    scanner = scannerPtr;
    debugFlag = dbg;
}

Parser::~Parser() {

}

/* Parse the entire program
 */
bool Parser::parse() {
    // Get the first token
    token = scanner->scan();

    // At least one program needed to compile
    if (!program()) {
        return false;
    }

    // Todo verify multiple programs per file allowed
    while (!isTokenType(T_EOF)) {
        if (!program()) {
            return false;
        }
    }
    return true;
}


void Parser::error(std::string msg) {
    errorFlag = true;
    scanner->error(msg);
}
void Parser::warning(std::string msg) {
    scanner->warning(msg);
}
void Parser::debug(std::string msg) {
    scanner->debug(msg);
}

/* 
 */
bool Parser::isTokenType(TokenType t) {
    if (t == token.type) {
        token = scanner->scan();
        return true;
    } else {
        return false;
    }
}

/* <program> ::= <program_header> <program_body> .
 */
bool Parser::program() {
    if (!programHeader()) {
        return false;
    }
    if (!programBody()) {
        return false;
    }
    if (!isTokenType(T_PERIOD)) {
        error("Missing \'.\' at end of program");
        return false;
    }
    return true;
}

/* <program_header> ::= program <identifier> is
 */
bool Parser::programHeader() {
    if (!isTokenType(T_PROGRAM)) {
        return false;
    }
    if (!identifier()) {
        return false;
    }
    if (!isTokenType(T_IS)) {
        error("Missing \'is\' keyword in program header");
        return false;
    }
    return true;
}

/* <program_body> ::=
 *          ( <declaration> ; )*
 *      begin
 *          ( <statement> ; )*
 *      end program
 */
bool Parser::programBody() {
    if (!declarationBlockHelper()) {
        return false;
    }
    if (!isTokenType(T_BEGIN)) {
        error("Missing \'begin\' keyword in program body");
        return false;
    }
    if (!statementBlockHelper()) {
        return false;
    }
    if (!isTokenType(T_END)) {
        error("Missing \'end\' keyword in program body");
        return false;
    }
    if (!isTokenType(T_PROGRAM)) {
        error("Missing \'program\' keyword in program body");
        return false;
    }
    return true;
}

/* <declaration> ::=
 *      [ global ] <procedure_declaration>
 *    | [ global ] <variable_declaration>
 *    | [ global ] <type_declaration>
 */
bool Parser::declaration() {
    if (isTokenType(T_GLOBAL)) {
        // Handle scoping
    }

    if (procedureDeclaration()) {

    } else if (variableDeclaration()) {

    } else if (typeDeclaration()) {

    } else {
        return false;
    }
    return true;
}


/* <procedure_declaration> ::= <procedure_header> <procedure_body>
 */
bool Parser::procedureDeclaration() {
    if (!procedureHeader()) {
        return false;
    }
    if (!procedureBody()) {
        return false;
    }
    return true;
}

/* <procedure_header> ::=
 *      procedure <identifier> : <type_mark> ( [<parameter_list>] )
 */
bool Parser::procedureHeader() {
    if (!isTokenType(T_PROCEDURE)) {
        return false;
    }
    if (!identifier()) {
        return false;
    }
    if (!isTokenType(T_COLON)) {
        error("Missing \':\' in procedure header");
        return false;
    }
    if (!typeMark()) {
        return false;
    }
    if (!isTokenType(T_LPAREN)) {
        error("Missing \'(\' in procedure header");
        return false;
    }

    // Optional
    parameterList();
    
    if (!isTokenType(T_RPAREN)) {
        error("Missing \')\' in procedure header");
        return false;
    }
    return true;
}

/* <parameter_list> ::=
 *      <parameter> , <parameter_list>
 *    | <parameter>
 */
bool Parser::parameterList() {
    if (!parameter()) {
        return false;
    }
    if (isTokenType(T_COMMA)) {
        return parameterList();
    }
    return true;
}

/* <parameter> ::= <variable_declaration>
 */
bool Parser::parameter() {
    return variableDeclaration();
}

/* <procedure_body> ::=
 *          ( <declaration> ; )*
 *      begin
 *          ( <statement> ; )*
 *      end procedure
 */
bool Parser::procedureBody() {
    if (!declarationBlockHelper()) {
        return false;
    }
    if (!isTokenType(T_BEGIN)) {
        error("Missing \'begin\' keyword in procedure body");
        return false;
    }
    if (!statementBlockHelper()) {
        return false;
    }
    if (!isTokenType(T_END)) {
        error("Missing \'end\' keyword in procedure body");
        return false;
    }
    if (!isTokenType(T_PROCEDURE)) {
        error("Missing \'procedure\' keyword in procedure body");
        return false;
    }
    return true;
}


/* <variable_declaration> ::=
 *      variable <identifier> : <type_mark> [ [ <bound> ] ]
 */
bool Parser::variableDeclaration() {
    if (!isTokenType(T_VARIABLE)) {
        return false;
    }
    if (!identifier()) {

    }
    if (!isTokenType(T_COLON)) {
        error("Missing \':\' in variable declaration");
        return false;
    }
    if (!typeMark()) {
        return false;
    }

    // Optional
    if (isTokenType(T_LBRACKET)) {
        if (!bound()) {
            return false;
        }
        if (!isTokenType(T_RBRACKET)) {
            error("Missing \']\' in variable bound");
            return false;
        }
    }
    return true;
}

/* <type_declaration> ::= type <identifier> is <type_mark>
 */
bool Parser::typeDeclaration() {
    if (!isTokenType(T_TYPE)) {
        return false;
    }
    if (!identifier()) {
        return false;
    }
    if (!isTokenType(T_IS)) {
        error("Missing \'is\' in type declaration");
        return false;
    }
    if (!typeMark()) {
        return false;
    }
    return true;
}

/* <type_mark>
 *      integer | float | string | bool
 *    | <identifier>
 *    | enum { <identifier> ( , <identifier> )* }
 */
bool Parser::typeMark() {
    if (isTokenType(T_INTEGER) ||
        isTokenType(T_FLOAT) ||
        isTokenType(T_STRING) ||
        isTokenType(T_BOOL)) {

    } else if (identifier()) {

    } else if (isTokenType(T_ENUM)) {
        if (!isTokenType(T_LBRACE)) {
            return false;
        }

        // At least one id required
        if (!identifier()) {
            error("No identifiers in enum");
            return false;
        }
        while (isTokenType(T_COMMA)) {
            if (!identifier()) {
                return false;
            }
        }

        if (!isTokenType(T_RBRACE)) {
            error("Missing \']\' in enum");
            return false;
        }
    } else {
        return false;
    }
    return true;
}

/* <bound> ::= <number>
 */
bool Parser::bound() {
    return number();
}


/* <statement> ::=
 *      <assignment_statement>
 *    | <if_statement>
 *    | <loop_statement>
 *    | <return_statement>
 */
bool Parser::statement() {
    if (assignmentStatement()) {

    } else if (ifStatement()) {

    } else if (loopStatement()) {

    } else if (returnStatement()) {

    } else {
        return false;
    }
    return true;
}

/* <procedure_call> ::= <identifier> ( [<argument_list>] )
 */
bool Parser::procedureCall() {
    if (!identifier()) {
        return false;
    }
    if (!isTokenType(T_LPAREN)) {
        return false;
    }

    // Optional
    argumentList();

    if (!isTokenType(T_RPAREN)) {
        error("Missing \')\' in procedure call");
        return false;
    }
    return true;
}

/* <assignment_statement> ::= <destination> := <expression>
 */
bool Parser::assignmentStatement() {
    if (!destination()) {
        return false;
    }
    if (!isTokenType(T_ASSIGNMENT)) {
        return false;
    }
    if (!expression()) {
        return false;
    }
    return true;
}

/* <destination> ::= <identifier> [ [ <expression> ] ]
 */
bool Parser::destination() {
    if (!identifier()) {
        return false;
    }

    // Optional
    if (isTokenType(T_LBRACKET)) {
        if (!expression()) {
            return false;
        }
        if (!isTokenType(T_RBRACKET)) {
            error("Missing \']\' in destination");
            return false;
        }
    }
    return true;
}

/* <if_statement> ::=
 *      if ( <expression> ) then ( <statement> ; )*
 *      [ else ( <statement> ; )* ]
 *      end if
 */
bool Parser::ifStatement() {
    if (!isTokenType(T_IF)) {
        return false;
    }
    if (!isTokenType(T_LPAREN)) {
        error("Missing \'(\' in if statement");
        return false;
    }
    if (!expression()) {
        return false;
    }
    if (!isTokenType(T_RPAREN)) {
        error("Missing \')\' in if statement");
        return false;
    }
    if (!isTokenType(T_THEN)) {
        error("Missing \'then\' in if statement");
        return false;
    }
    if (!statementBlockHelper()) {
        return false;
    }

    if (isTokenType(T_ELSE)) {
        if (!statementBlockHelper()) {
            return false;
        }
    }

    if (!isTokenType(T_END)) {
        error("Missing \'end\' in if statement");
        return false;
    }
    if (!isTokenType(T_IF)) {
        error("Missing closing \'if\'");
        return false;
    }
    return true;
}

/* <loop_statement> ::=
 *      for ( <assignment_statement> ; <expression> )
 *          ( <statement> ; )*
 *      end for
 */
bool Parser::loopStatement() {
    if (!isTokenType(T_FOR)) {
        return false;
    }
    if (!isTokenType(T_LPAREN)) {
        error("Missing \'(\' in loop");
        return false;
    }
    if (!assignmentStatement()) {
        return false;
    }
    if (!isTokenType(T_SEMICOLON)) {
        error("Missing \':\' in loop");
        return false;
    }
    if (!expression()) {
        return false;
    }
    if (!isTokenType(T_RPAREN)) {
        error("Missing \')\' in loop");
        return false;
    }

    if (!statementBlockHelper()) {
        return false;
    }
    if (!isTokenType(T_END)) {
        error("Missing \'end\' in loop");
        return false;
    }
    if (!isTokenType(T_FOR)) {
        error("Missing closing \'for\' in loop");
        return false;
    }
    return true;
}

/* <return_statement> ::= return <expression>
 */
bool Parser::returnStatement() {
    if (!isTokenType(T_RETURN)) {
        return false;
    }
    if (!expression()) {
        return false;
    }
    return true;
}


/* <identifier> ::= [a-zA-Z][a-zA-Z0-9_]*
 */
bool Parser::identifier() {
    return isTokenType(T_IDENTIFIER);
}

/* <expression> ::= [ not ] <arithOp> <expression_prime>
 */
bool Parser::expression() {
    isTokenType(T_NOT);
    if (!arithOp()) {
        return false;
    }
    if (!expression_p()) {
        return false;
    }
    return true;
}

/* <expression_prime> ::=
 *      & <arithOp> <expression_prime>
 *    | | <arithOp> <expression_prime>
 *    | null
 */
bool Parser::expression_p() {
    if (isTokenType(T_AND) ||
        isTokenType(T_OR)) {
        if (!arithOp()) {
            return false;
        }
        if (!expression_p()) {
            return false;
        }
    }
    return true;
}

/* <arithOp> ::= <relation> <arithOp_prime>
 */
bool Parser::arithOp() {
    if (!relation()) {
        return false;
    }
    if (!arithOp_p()) {
        return false;
    }
    return true;
}

/* <arithOp_prime> ::=
 *      + <relation> <arithOp_prime>
 *    | - <relation> <arithOp_prime>
 *    | null
 */
bool Parser::arithOp_p() {
    if (isTokenType(T_PLUS) ||
        isTokenType(T_MINUS)) {
        if (!relation()) {
            return false;
        }
        if (!arithOp_p()) {
            return false;
        }
    }
    return true;
}

/* <relation> ::= <term> <relation_prime>
 */
bool Parser::relation() {
    if (!term()) {
        return false;
    }
    if (!relation_p()) {
        return false;
    }
    return true;
}

/* <relation_prime> ::=
 *      <  <term> <relation_prime>
 *    | >= <term> <relation_prime>
 *    | <= <term> <relation_prime>
 *    | >  <term> <relation_prime>
 *    | == <term> <relation_prime>
 *    | != <term> <relation_prime>
 *    | null
 */
bool Parser::relation_p() {
    if (isTokenType(T_LESS) ||
        isTokenType(T_GREATER_EQ) ||
        isTokenType(T_LESS_EQ) ||
        isTokenType(T_GREATER) ||
        isTokenType(T_EQUAL) ||
        isTokenType(T_NOT_EQUAL)) {
        if (!term()) {
            return false;
        }
        if (!relation_p()) {
            return false;
        }
    }
    return true;
}

/* <term> ::= <factor> <term_prime>
 */
bool Parser::term() {
    if (!factor()) {
        return false;
    }
    if (!term_p()) {
        return false;
    }
    return true;
}

/* <term_prime> ::=
 *      * <factor> <term_prime>
 *    | / <factor> <term_prime>
 *    | null
 */
bool Parser::term_p() {
    if (isTokenType(T_MULTIPLY) ||
        isTokenType(T_DIVIDE)) {
        if (!factor()) {
            return false;
        }
        if (!term_p()) {
            return false;
        }
    }
    return true;
}

/* <factor> ::=
 *      ( <expression> )
 *    | <procedure_call>
 *    | [ - ] <name>
 *    | [ - ] <number>
 *    | <string>
 *    | true
 *    | false
 */
bool Parser::factor() {
    if (isTokenType(T_LPAREN)) {
        if (!expression()) {
            return false;
        }
        if (!isTokenType(T_RPAREN)) {
            error("Missing \')\' in expression factor");
            return false;
        }
    } else if (procedureCall()) {
        // BROKEN
        // Both procedure call and name start with identifier

    } else if (isTokenType(T_MINUS) ||
               name()) {
        
    } else if (isTokenType(T_MINUS) ||
               number()) {
        
    } else if (isTokenType(T_STRING_VAL)) {
        
    } else if (isTokenType(T_TRUE) ||
               isTokenType(T_FALSE)) {

    } else {
        return false;
    }
    return true;
}

/* <name> ::= <identifier> [ [ <expression> ] ]
 */
bool Parser::name() {
    if (!identifier()) {
        return false;
    }
    if (isTokenType(T_LBRACKET)) {
        if (!expression()) {

            return false;
        }
        if (!isTokenType(T_RBRACKET)) {
            error("Missing \']\' in name");
            return false;
        }
    }
    return true;
}

/* <argument_list> ::=
 *      <expression> , <argument_list>
 *    | <expression>
 */
bool Parser::argumentList() {
    if (!expression()) {
        return false;
    }
    if (isTokenType(T_COMMA)) {
        return argumentList();
    }
    return true;
}

/* <number> ::= [0-9][0-9_]*[.[0-9_]*]
 */
bool Parser::number() {
    return isTokenType(T_INTEGER_VAL) || isTokenType(T_FLOAT_VAL);
}

/* <string> :: = "[^"]*"
 */
bool Parser::string() {
    return isTokenType(T_STRING_VAL);
}


/* Helper for ( <declaration> ; )*
 */
bool Parser::declarationBlockHelper() {
    // At least one declaration required
    if (!declaration()) {
        return false;
    }
    if (!isTokenType(T_SEMICOLON)) {
        error("Missing \';\' after declaration");
        return false;
    }

    while (declaration()) {
        if (!isTokenType(T_SEMICOLON)) {
            error("Missing \';\' after declaration");
            return false;
        }
    }
    return true;
}

/* Helper for ( <statement> ; )*
 */
bool Parser::statementBlockHelper() {
    // At least one statement required
    if (!statement()) {
        return false;
    }
    if (!isTokenType(T_SEMICOLON)) {
        error("Missing \';\' after statement");
        return false;
    }

    while (statement()) {
        if (!isTokenType(T_SEMICOLON)) {
            error("Missing \';\' after statement");
            return false;
        }
    }
    return true;
}