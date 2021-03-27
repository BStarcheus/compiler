#include "parser.h"
#include "token.h"
#include <iostream>

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"


Parser::Parser(Scanner* scannerPtr, ScopeManager* scoperPtr, bool dbg) {
    scanner = scannerPtr;
    scoper = scoperPtr;
    debugFlag = dbg;
    errorFlag = false;

    llvm_context = new llvm::LLVMContext();
}

Parser::~Parser() {
    delete llvm_builder;
    delete llvm_module;
    delete llvm_context;
}

/* Parse the entire program
 */
bool Parser::parse() {
    // Get the first token
    token = scanner->scan();
    return program();
}


/* Output the LLVM Module to .s assembly file
 */
void Parser::outputAssembly() {
    // Initialize the target registry etc.
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    llvm_module->setTargetTriple(targetTriple);

    std::string err;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, err);

    // If no target found
    if (!target) {
        llvm::errs() << err;
        return;
    }

    std::string cpu = "generic";
    std::string features = "";

    llvm::TargetOptions opt;
    auto rm = llvm::Optional<llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, rm);

    llvm_module->setDataLayout(targetMachine->createDataLayout());

    std::string filename = "out.s";
    std::error_code errCode;
    llvm::raw_fd_ostream dest(filename, errCode, llvm::sys::fs::OF_None);

    if (errCode) {
        llvm::errs() << "Could not open output file: " << errCode.message();
        return;
    }

    llvm::legacy::PassManager pm;
    auto fileType = llvm::CGFT_AssemblyFile;

    if (targetMachine->addPassesToEmitFile(pm, dest, nullptr, fileType)) {
        llvm::errs() << "TargetMachine cannot emit a file of this type.";
        return;
    }

    pm.run(*llvm_module);
    dest.flush();
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
    if (token.type == t) {
        token = scanner->scan();
        return true;
    } else {
        return false;
    }
}

/* <program> ::= <program_header> <program_body> .
 */
bool Parser::program() {
    // Outermost/global scope is already set in ScopeManager init
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
    if (!isTokenType(T_EOF)) {
        return false;
    }
    scoper->exitScope();
    return true;
}

/* <program_header> ::= program <identifier> is
 */
bool Parser::programHeader() {
    if (!isTokenType(T_PROGRAM)) {
        return false;
    }
    Symbol id;
    if (!identifier(id)) {
        error("Invalid identifier \'" + id.id + "\'");
        return false;
    }

    // Create LLVM module with program name
    llvm_module = new llvm::Module(id.id, *llvm_context);
    llvm_builder = new llvm::IRBuilder<>(*llvm_context);

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
 */
bool Parser::declaration() {
    Symbol decl;
    decl.isGlobal = isTokenType(T_GLOBAL);

    if (procedureDeclaration(decl)) {

    } else if (variableDeclaration(decl)) {

    } else {
        return false;
    }
    return true;
}


/* <procedure_declaration> ::= <procedure_header> <procedure_body>
 */
bool Parser::procedureDeclaration(Symbol &decl) {
    if (!procedureHeader(decl)) {
        return false;
    }

    decl.symbolType = ST_PROCEDURE;

    // Error if duplicate name within function scope
    if (scoper->hasSymbol(decl.id, decl.isGlobal)) {
        error("Procedure name \'" + decl.id + "\' already used in this scope");
        return false;
    }
    // Set inside function, so recursive calls possible
    scoper->setSymbol(decl.id, decl, decl.isGlobal);

    // Set inside function so proc symbol can be easily found
    // for return type checking
    scoper->setCurrentProcedure(decl);


    if (!procedureBody()) {
        return false;
    }
    scoper->exitScope();

    // If global, already added to global table above
    if (!decl.isGlobal) {
        // Error if duplicate name in local scope outside the function
        if (scoper->hasSymbol(decl.id, decl.isGlobal)) {
            error("Procedure name \'" + decl.id + "\' already used in this scope");
            return false;
        }
        // Set in local scope outside the function
        scoper->setSymbol(decl.id, decl, decl.isGlobal);
    }

    return true;
}

/* <procedure_header> ::=
 *      procedure <identifier> : <type_mark> ( [<parameter_list>] )
 */
bool Parser::procedureHeader(Symbol &decl) {
    if (!isTokenType(T_PROCEDURE)) {
        return false;
    }

    scoper->newScope();
    if (!identifier(decl)) {
        error("Invalid identifier \'" + decl.id + "\'");
        return false;
    }

    if (!isTokenType(T_COLON)) {
        error("Missing \':\' in procedure header");
        return false;
    }
    if (!typeMark(decl)) {
        error("Invalid type mark");
        return false;
    }
    if (!isTokenType(T_LPAREN)) {
        error("Missing \'(\' in procedure header");
        return false;
    }

    // Optional
    parameterList(decl);
    if (errorFlag) {
        return false;
    }
    
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
bool Parser::parameterList(Symbol &decl) {
    Symbol param;
    if (!parameter(param)) {
        return false;
    }
    // Add to procedure param list
    decl.params.push_back(param);

    // Optional
    while (isTokenType(T_COMMA)) {
        param = Symbol();
        if (!parameter(param)) {
            error("Invalid parameter");
            return false;
        }
        // Add to procedure param list
        decl.params.push_back(param);
    }
    return true;
}

/* <parameter> ::= <variable_declaration>
 */
bool Parser::parameter(Symbol &param) {
    return variableDeclaration(param);
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
bool Parser::variableDeclaration(Symbol &decl) {
    if (!isTokenType(T_VARIABLE)) {
        return false;
    }

    decl.symbolType = ST_VARIABLE;

    if (!identifier(decl)) {
        error("Invalid identifier \'" + decl.id + "\'");
        return false;
    }

    // Error if duplicate name within scope
    if (scoper->hasSymbol(decl.id, decl.isGlobal)) {
        error("Variable name \'" + decl.id + "\' already used in this scope");
        return false;
    }
    
    if (!isTokenType(T_COLON)) {
        error("Missing \':\' in variable declaration");
        return false;
    }
    if (!typeMark(decl)) {
        error("Invalid type mark");
        return false;
    }

    // Optional
    if (isTokenType(T_LBRACKET)) {
        if (!bound(decl)) {
            error("Invalid bound");
            return false;
        }

        decl.isArr = true;

        if (!isTokenType(T_RBRACKET)) {
            error("Missing \']\' in variable bound");
            return false;
        }
    }

    // Set in scope
    scoper->setSymbol(decl.id, decl, decl.isGlobal);
    
    return true;
}


/* <type_mark> ::=
 *      integer | float | string | bool
 */
bool Parser::typeMark(Symbol &id) {
    if (isTokenType(T_INTEGER)) {
        id.type = TYPE_INT;
    } else if (isTokenType(T_FLOAT)) {
        id.type = TYPE_FLOAT;
    } else if (isTokenType(T_STRING)) {
        id.type = TYPE_STRING;
    } else if (isTokenType(T_BOOL)) {
        id.type = TYPE_BOOL;
    } else {
        return false;
    }
    return true;
}

/* <bound> ::= <number>
 */
bool Parser::bound(Symbol &id) {
    Symbol num;

    // TODO: Change impl with LLVM
    int temp = token.getIntVal();

    if (number(num) && num.type == TYPE_INT) {
        id.arrSize = temp;
        return true;
    } else {
        error("Invalid bound. Must be an integer.");
        return false;
    }
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

/* <assignment_statement> ::= <destination> := <expression>
 */
bool Parser::assignmentStatement() {
    Symbol dest, exp;

    if (!destination(dest)) {
        return false;
    }
    if (!isTokenType(T_ASSIGNMENT)) {
        return false;
    }
    if (!expression(exp)) {
        return false;
    }

    // Type check
    if (!compatibleTypeCheck(dest, exp)) {
        return false;
    }

    return true;
}

/* <destination> ::= <identifier> [ [ <expression> ] ]
 */
bool Parser::destination(Symbol &id) {
    if (!identifier(id)) {
        return false;
    }

    // Error if identifier is not in local or global scope
    if (!scoper->hasSymbol(id.id)) {
        error("\'" + id.id + "\' not declared in scope");
        return false;
    }
    // Get from local or global
    id = scoper->getSymbol(id.id);

    if (!arrayIndexHelper(id)) {
        return false;
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
    Symbol exp;
    if (!expression(exp)) {
        return false;
    }

    // Type check/convert to bool
    if (exp.type == TYPE_INT) {
        exp.type = TYPE_BOOL;

    } else if (exp.type != TYPE_BOOL) {
        error("If statement expressions must evaluate to bool");
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

    // Optional
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
    Symbol exp;
    if (!expression(exp)) {
        return false;
    }

    // Type check/convert to bool
    if (exp.type == TYPE_INT) {
        exp.type = TYPE_BOOL;

    } else if (exp.type != TYPE_BOOL) {
        error("Loop statement expressions must evaluate to bool");
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
    Symbol exp;
    if (!expression(exp)) {
        return false;
    }

    // Type check match to procedure return type
    Symbol proc = scoper->getCurrentProcedure();
    if (proc.type == TYPE_UNK) {
        error("Return statements must be within a procedure");
        return false;
    } else if (!compatibleTypeCheck(proc, exp)) {
        return false;
    }

    return true;
}


/* <identifier> ::= [a-zA-Z][a-zA-Z0-9_]*
 */
bool Parser::identifier(Symbol &id) {
    // Check without consuming
    if (token.type == T_IDENTIFIER) {
        id.id = token.val;
        id.tokenType = token.type;
    }
    // Consume token
    return isTokenType(T_IDENTIFIER);
}

/* <expression> ::= [ not ] <arithOp> <expression_prime>
 */
bool Parser::expression(Symbol &exp) {
    // Optional
    bool nt = isTokenType(T_NOT);

    if (!arithOp(exp)) {
        return false;
    }

    // Type check for not op
    // Only valid for bool and int
    if (nt) {
        if (exp.type != TYPE_BOOL && exp.type != TYPE_INT) {
            error("\'not\' operator only defined for bool and int");
        }
    }

    if (!expression_p(exp)) {
        return false;
    }
    return true;
}

/* <expression_prime> ::=
 *      & <arithOp> <expression_prime>
 *    | | <arithOp> <expression_prime>
 *    | null
 */
bool Parser::expression_p(Symbol &exp) {
    if (isTokenType(T_AND) ||
        isTokenType(T_OR)) {
        Symbol rhs;
        if (!arithOp(rhs)) {
            return false;
        }

        // Check/convert type for & |
        expressionTypeCheck(exp, rhs);

        if (!expression_p(exp)) {
            return false;
        }
    }
    return true;
}

/* <arithOp> ::= <relation> <arithOp_prime>
 */
bool Parser::arithOp(Symbol &arOp) {
    if (!relation(arOp)) {
        return false;
    }
    if (!arithOp_p(arOp)) {
        return false;
    }
    return true;
}

/* <arithOp_prime> ::=
 *      + <relation> <arithOp_prime>
 *    | - <relation> <arithOp_prime>
 *    | null
 */
bool Parser::arithOp_p(Symbol &arOp) {
    if (isTokenType(T_PLUS) ||
        isTokenType(T_MINUS)) {
        Symbol rhs;
        if (!relation(rhs)) {
            return false;
        }

        // Check/convert type for + -
        if (!arithmeticTypeCheck(arOp, rhs)) {
            return false;
        }

        if (!arithOp_p(arOp)) {
            return false;
        }
    }
    return true;
}

/* <relation> ::= <term> <relation_prime>
 */
bool Parser::relation(Symbol &rel) {
    if (!term(rel)) {
        return false;
    }
    if (!relation_p(rel)) {
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
bool Parser::relation_p(Symbol &rel) {
    Token op = token;
    if (isTokenType(T_LESS) ||
        isTokenType(T_GREATER_EQ) ||
        isTokenType(T_LESS_EQ) ||
        isTokenType(T_GREATER) ||
        isTokenType(T_EQUAL) ||
        isTokenType(T_NOT_EQUAL)) {
        Symbol rhs;
        if (!term(rhs)) {
            return false;
        }

        // Check/convert type for rel ops
        if (!relationTypeCheck(rel, rhs, op)) {
            return false;
        }

        // Compatible relation evaluates to bool
        rel.type = TYPE_BOOL;

        if (!relation_p(rel)) {
            return false;
        }
    }
    return true;
}

/* <term> ::= <factor> <term_prime>
 */
bool Parser::term(Symbol &trm) {
    if (!factor(trm)) {
        return false;
    }
    if (!term_p(trm)) {
        return false;
    }
    return true;
}

/* <term_prime> ::=
 *      * <factor> <term_prime>
 *    | / <factor> <term_prime>
 *    | null
 */
bool Parser::term_p(Symbol &trm) {
    if (isTokenType(T_MULTIPLY) ||
        isTokenType(T_DIVIDE)) {
        Symbol rhs;
        if (!factor(rhs)) {
            return false;
        }

        // Check/convert type for * /
        if (!arithmeticTypeCheck(trm, rhs)) {
            return false;
        }

        if (!term_p(trm)) {
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
bool Parser::factor(Symbol &fac) {
    if (isTokenType(T_LPAREN)) {
        if (!expression(fac)) {
            return false;
        }
        if (!isTokenType(T_RPAREN)) {
            error("Missing \')\' in expression factor");
            return false;
        }
    } else if (procCallOrName(fac)) {
        // Both procedure call and name start with identifier

    } else if (isTokenType(T_MINUS)) {
        if (name(fac)) {

        } else if (number(fac)) {

        } else {
            error("Invalid use of \'-\'");
            return false;
        }
    } else if (number(fac)) {
        
    } else if (string(fac)) {
        
    } else if (isTokenType(T_TRUE)) {
        fac.tokenType = T_TRUE;
        fac.type = TYPE_BOOL;
    } else if (isTokenType(T_FALSE)) {
        fac.tokenType = T_FALSE;
        fac.type = TYPE_BOOL;
    } else {
        return false;
    }
    return true;
}

/* Helper to handle procedure call or name in factor
 *
 * <procedure_call> ::= <identifier> ( [<argument_list>] )
 *
 * <name> ::= <identifier> [ [ <expression> ] ]
 */
bool Parser::procCallOrName(Symbol &id) {
    if (!identifier(id)) {
        return false;
    }

    // Error if identifier is not in local or global scope
    if (!scoper->hasSymbol(id.id)) {
        error("\'" + id.id + "\' not declared in scope");
        return false;
    }
    // Get from local or global
    id = scoper->getSymbol(id.id);

    if (isTokenType(T_LPAREN)) {
        // Procedure call

        // Optional
        argumentList(id);
        if (errorFlag) {
            return false;
        }

        if (!isTokenType(T_RPAREN)) {
            error("Missing \')\' in procedure call");
            return false;
        }
    } else {
        // Name
        // Check if array access
        if (!arrayIndexHelper(id)) {
            return false;
        }
    }
    return true;
}

/* <name> ::= <identifier> [ [ <expression> ] ]
 */
bool Parser::name(Symbol &id) {
    // This function is kept because in some cases we don't want
    // a procedure call to be valid, only a name

    if (!identifier(id)) {
        return false;
    }

    // Error if identifier is not in local or global scope
    if (!scoper->hasSymbol(id.id)) {
        error("\'" + id.id + "\' not declared in scope");
        return false;
    }
    // Get from local or global
    id = scoper->getSymbol(id.id);

    if (!arrayIndexHelper(id)) {
        return false;
    }
    return true;
}

/* Handle array access index
 * [ [ <expression> ] ]
 */
bool Parser::arrayIndexHelper(Symbol &id) {
    // Optional
    if (isTokenType(T_LBRACKET)) {
        Symbol exp;
        if (!expression(exp)) {
            return false;
        }

        // Check valid access
        if (!id.isArr) {
            error("\'" + id.id + "\' is not an array, and cannot be indexed");
            return false;
        } else if (exp.type != TYPE_INT) {
            error("Array index must be type integer");
            return false;
        }
        // TODO Codegen: check exp value < id.arrSize

        // Identifier is indexed
        id.isIndexed = true;

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
bool Parser::argumentList(Symbol &id) {
    Symbol arg;
    int argInd = 0;
    if (!expression(arg)) {
        if (argInd != id.params.size()) {
            error("Too few arguments provided to \'" + id.id + "\'");
        }
        return false;
    }

    // Check number of params
    if (argInd >= id.params.size()) {
        error("Too many arguments provided to \'" + id.id + "\'");
        return false;

    // Check type match to param
    } else if (!compatibleTypeCheck(id.params[argInd], arg)) {
        return false;
    }
    argInd++;


    // Optional
    while (isTokenType(T_COMMA)) {
        arg = Symbol();
        if (!expression(arg)) {
            error("Invalid argument");
            return false;
        }

        // Check number of params
        if (argInd >= id.params.size()) {
            error("Too many arguments provided to \'" + id.id + "\'");
            return false;

        // Check type match to param
        } else if (!compatibleTypeCheck(id.params[argInd], arg)) {
            return false;
        }
        argInd++;
    }

    // Check number of params
    if (argInd != id.params.size()) {
        error("Too few arguments provided to \'" + id.id + "\'");
        return false;
    }
    return true;
}

/* <number> ::= [0-9][0-9_]*[.[0-9_]*]
 */
bool Parser::number(Symbol &num) {
    // Check without consuming
    if (token.type == T_INTEGER_VAL) {
        num.type = TYPE_INT;
        num.tokenType = T_INTEGER_VAL;
        
        // Consume token
        return isTokenType(T_INTEGER_VAL);

    //Check without consuming
    } else if (token.type == T_FLOAT_VAL) {
        num.type = TYPE_FLOAT;
        num.tokenType = T_FLOAT_VAL;

        // Consume token
        return isTokenType(T_FLOAT_VAL);
    } else {
        return false;
    }
}

/* <string> :: = "[^"]*"
 */
bool Parser::string(Symbol &str) {
    // Check without consuming
    if (token.type == T_STRING_VAL) {
        str.id = token.val;
        str.tokenType = token.type;
        str.type = TYPE_STRING;
    }
    // Consume token
    return isTokenType(T_STRING_VAL);
}


/* Helper for ( <declaration> ; )*
 */
bool Parser::declarationBlockHelper() {
    // Zero or more declarations
    while (declaration()) {
        if (!isTokenType(T_SEMICOLON)) {
            error("Missing \';\' after declaration");
            return false;
        }
    }
    return !errorFlag;
}

/* Helper for ( <statement> ; )*
 */
bool Parser::statementBlockHelper() {
    // Zero or more statements
    while (statement()) {
        if (!isTokenType(T_SEMICOLON)) {
            error("Missing \';\' after statement");
            return false;
        }
    }
    return !errorFlag;
}


/* Type checking for arithmetic operators + - * / 
 */
bool Parser::arithmeticTypeCheck(Symbol &lhs, Symbol &rhs) {
    if ((lhs.type != TYPE_INT && lhs.type != TYPE_FLOAT) ||
        (rhs.type != TYPE_INT && rhs.type != TYPE_FLOAT)) {
        error("Arithmetic only defined for int and float");
        return false;
    }
    
    if (lhs.type == TYPE_INT) {
        if (rhs.type == TYPE_FLOAT) {
            // Convert lhs to float
            lhs.type = TYPE_FLOAT;
        }
        // Else both int, types match
        
    } else { // lhs is float
        if (rhs.type == TYPE_INT) {
            // Convert rhs to float
            rhs.type = TYPE_FLOAT;
        }
        // Else both float, types match
    }
    return true;
}

/* Type checking for relational operators < <= > >= == !=
 */
bool Parser::relationTypeCheck(Symbol &lhs, Symbol &rhs, Token &op) {
    bool compatible = false;
    // If int is present with float or bool, convert int to that type
    // Otherwise types must match exactly
    
    if (lhs.type == TYPE_INT) {
        if (rhs.type == TYPE_BOOL) {
            compatible = true;
            // Convert lhs to bool
            lhs.type = TYPE_BOOL;

        } else if (rhs.type == TYPE_FLOAT) {
            compatible = true;
            // Convert lhs to float
            lhs.type = TYPE_FLOAT;

        } else if (rhs.type == TYPE_INT) {
            compatible = true;
        }

    } else if (lhs.type == TYPE_FLOAT) {
        if (rhs.type == TYPE_FLOAT) {
            compatible = true;

        } else if (rhs.type == TYPE_INT) {
            compatible = true;
            // Convert rhs to float
            rhs.type = TYPE_FLOAT;
        }

    } else if (lhs.type == TYPE_BOOL) {
        if (rhs.type == TYPE_BOOL) {
            compatible = true;

        } else if (rhs.type == TYPE_INT) {
            compatible = true;
            // Convert rhs to bool
            rhs.type = TYPE_BOOL;
        }

    } else if (lhs.type == TYPE_STRING) {
        // Strings only valid for == !=
        if (rhs.type == TYPE_STRING && 
            (op.type == T_EQUAL || op.type == T_NOT_EQUAL)) {
            compatible = true;
        }
    }

    if (!compatible) {
        error("Incompatible relation operands");
    }
    return compatible;
}

/* Type checking for expression operators & |
 */
bool Parser::expressionTypeCheck(Symbol &lhs, Symbol &rhs) {
    bool compatible = false;
    
    if (lhs.type == TYPE_BOOL && rhs.type == TYPE_BOOL) {
        compatible = true;
    } else if (lhs.type == TYPE_INT && rhs.type == TYPE_INT) {
        compatible = true;
    }

    if (!compatible) {
        error("Expression ops only defined for bool and int");
    }
    return compatible;
}


/* Type checking for assignment operator =
 * and matching params to arguments
 */
bool Parser::compatibleTypeCheck(Symbol &dest, Symbol &exp) {
    bool compatible = false;
    // If types are compatible, convert exp to type of dest
    // int <-> bool
    // int <-> float
    // Otherwise types must match exactly
    
    if (dest.type == exp.type) {
        compatible = true;
    } else if (dest.type == TYPE_INT) {
        if (exp.type == TYPE_BOOL) {
            compatible = true;
            // Convert exp to int
            exp.type = TYPE_INT;

        } else if (exp.type == TYPE_FLOAT) {
            compatible = true;
            // Convert exp to int
            exp.type = TYPE_INT;
        }
    } else if (dest.type == TYPE_FLOAT) {
        if (exp.type == TYPE_INT) {
            compatible = true;
            // Convert exp to float
            exp.type = TYPE_FLOAT;
        }
    } else if (dest.type == TYPE_BOOL) {
        if (exp.type == TYPE_INT) {
            compatible = true;
            // Convert exp to bool
            exp.type = TYPE_BOOL;
        }
    }

    if (!compatible) {
        error("Incompatible types " + getTypeName(dest.type) + " and " + getTypeName(exp.type));
    }

    /* Check valid matching of isArr isIndexed
     * Valid:
     * var = var
     * arr = arr
     * arr[i] = arr[i]
     * arr[i] = var
     * var = arr[i]
     */
    if (dest.isArr || exp.isArr) {
        if (dest.isArr && exp.isArr) {
            // Both arrays

            if (dest.isIndexed != exp.isIndexed) {
                error("Incompatible index match of arrays");
                compatible = false;
            } else if (!dest.isIndexed) {
                // Both are unindexed. Array lengths must match
                if (dest.arrSize != exp.arrSize) {
                    error("Array lengths must match");
                    compatible = false;
                }
            }
            // Else both are indexed

        } else {
            // One side is array
            // Array must be indexed
            if ((dest.isArr && !dest.isIndexed) ||
                (exp.isArr && !exp.isIndexed)) {
                error("Array is not indexed");
                compatible = false;
            }
        }
    }
    // Else both are non-array

    return compatible;
}