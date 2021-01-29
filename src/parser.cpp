#include "parser.h"
#include "tokenType.h"

Parser::Parser() {

}

Parser::~Parser() {

}

/* Parse the entire program
 */
bool Parser::parse() {
    return program();
}


/* <program> ::= <program_header> <program_body> .
 */
bool Parser::program() {
    return false;
}

/* <program_header> ::= program <identifier> is
 */
bool Parser::programHeader() {
    return false;
}

/* <program_body> ::=
 *          ( <declaration> ; )*
 *      begin
 *          ( <statement> ; )*
 *      end program
 */
bool Parser::programBody() {
    return false;
}

/* <declaration> ::=
 *      [ global ] <procedure_declaration>
 *    | [ global ] <variable_declaration>
 *    | [ global ] <type_declaration>
 */
bool Parser::declaration() {
    return false;
}


/* <procedure_declaration> ::= <procedure_header> <procedure_body>
 */
bool Parser::procedureDeclaration() {
    return false;
}

/* <procedure_header> ::=
 *      procedure <identifier> : <type_mark> ( [<parameter_list>] )
 */
bool Parser::procedureHeader() {
    return false;
}

/* <parameter_list> ::=
 *      <parameter> , <parameter_list>
 *    | <parameter>
 */
bool Parser::parameterList() {
    return false;
}

/* <parameter> ::= <variable_declaration>
 */
bool Parser::parameter() {
    return false;
}

/* <procedure_body> ::=
 *          ( <declaration> ; )*
 *      begin
 *          ( <statement> ; )*
 *      end procedure
 */
bool Parser::procedureBody() {
    return false;
}


/* <variable_declaration> ::=
 *      variable <identifier>: <type_mark> [ [ <bound> ] ]
 */
bool Parser::variableDeclaration() {
    return false;
}

/* <type_declaration> ::= type <identifier> is <type_mark>
 */
bool Parser::typeDeclaration() {
    return false;
}

/* <type_mark>
 *      integer | float | string | bool
 *    | <identifier>
 *    | enum { <identifier> ( , <identifier> )* }
 */
bool Parser::typeMark() {
    return false;
}

/* <bound> ::= <number>
 */
bool Parser::bound() {
    return false;
}


/* <statement> ::=
 *      <assignment_statement>
 *    | <if_statement>
 *    | <loop_statement>
 *    | <return_statement>
 */
bool Parser::statement() {
    return false;
}

/* <procedure_call> ::= <identifier> ( [<argument_list>] )
 */
bool Parser::precedureCall() {
    return false;
}

/* <assignment_statement> ::= <destination> := <expression>
 */
bool Parser::assignmentStatement() {
    return false;
}

/* <destination> ::= <identifier> [ [ <expression> ] ]
 */
bool Parser::destination() {
    return false;
}

/* <if_statement> ::=
 *      if ( <expression> ) then ( <statement> ; )*
 *      [ else ( <statement> ; )* ]
 *      end if
 */
bool Parser::ifStatement() {
    return false;
}

/* <loop_statement> ::=
 *      for ( <assignment_statement> ; <expression> )
 *          ( <statement> ; )*
 *      end for
 */
bool Parser::loopStatement() {
    return false;
}

/* <return_statement> ::= return <expression>
 */
bool Parser::returnStatement() {
    return false;
}


/* <identifier> ::= [a-zA-Z][a-zA-Z0-9_]*
 */
bool Parser::identifier() {
    return false;
}

/* <expression> ::= [ not ] <arithOp> [ <expression_prime> ]
 */
bool Parser::expression() {
    return false;
}

/* <expression_prime> ::=
 *      & <arithOp> [ <expression_prime> ]
 *    | | <arithOp> [ <expression_prime> ]
 */
bool Parser::expression_p() {
    return false;
}

/* <arithOp> ::=
 *      <relation> [ <arithOp_prime> ]
 */
bool Parser::arithOp() {
    return false;
}

/* <arithOp_prime> ::=
 *      + <relation> [ <arithOp_prime> ]
 *    | - <relation> [ <arithOp_prime> ]
 */
bool Parser::arithOp_p() {
    return false;
}

/* <relation> ::=
 *      <term> [ <relation_prime> ]
 */
bool Parser::relation() {
    return false;
}

/* <relation_prime> ::=
 *      <  <term> [ <relation_prime> ]
 *    | >= <term> [ <relation_prime> ]
 *    | <= <term> [ <relation_prime> ]
 *    | >  <term> [ <relation_prime> ]
 *    | == <term> [ <relation_prime> ]
 *    | != <term> [ <relation_prime> ]
 */
bool Parser::relation_p() {
    return false;
}

/* <term> ::=
 *      <factor> [ <term_prime> ]
 */
bool Parser::term() {
    return false;
}

/* <term_prime> ::=
 *      * <factor> [ <term_prime> ]
 *    | / <factor> [ <term_prime> ]
 */
bool Parser::term_p() {
    return false;
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
    return false;
}

/* <name> ::= <identifier> [ [ <expression> ] ]
 */
bool Parser::name() {
    return false;
}

/* <argument_list> ::=
 *      <expression> , <argument_list>
 *    | <expression>
 */
bool Parser::argumentList() {
    return false;
}

/* <number> ::= [0-9][0-9_]*[.[0-9_]*]
 */
bool Parser::number() {
    return false;
}

/* <string> :: = "[^"]*"
 */
bool Parser::string() {
    return false;
}