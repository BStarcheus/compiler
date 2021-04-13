#include "parser.h"
#include "token.h"
#include <iostream>

#include "llvm/IR/Constants.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"


Parser::Parser(Scanner* scannerPtr, ScopeManager* scoperPtr, bool dbgParser, bool dbgCodegen) {
    scanner = scannerPtr;
    scoper = scoperPtr;
    debugFlag = dbgParser;
    debugCodegenFlag = dbgCodegen;
    errorFlag = false;

    llvm_context = new llvm::LLVMContext();
    llvm_builder = new llvm::IRBuilder<>(*llvm_context);
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
    bool success = program();

    if (success) {
        debug("Parse succeeded");
    } else {
        debug("Parse failed");
    }
    return success;
}


/* Output the LLVM Module to .s assembly file
 */
bool Parser::outputAssembly() {
    bool invalid = llvm::verifyModule(*llvm_module, &llvm::errs());
    if (invalid) {
        error("Errors found in Module");
        if (debugCodegenFlag) {
            llvm_module->print(llvm::outs(), nullptr);
        }
        return false;
    }

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
        return false;
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
        return false;
    }

    llvm::legacy::PassManager pm;
    auto fileType = llvm::CGFT_AssemblyFile;

    if (targetMachine->addPassesToEmitFile(pm, dest, nullptr, fileType)) {
        llvm::errs() << "TargetMachine cannot emit a file of this type.";
        return false;
    }


    if (debugCodegenFlag) {
        // Emit IR for debugging
        std::string filename2 = "out.ll";
        std::error_code errCode2;
        llvm::raw_fd_ostream dest2(filename2, errCode2, llvm::sys::fs::OF_None);

        if (errCode2) {
            llvm::errs() << "Could not open output file: " << errCode2.message();
            return false;
        }
        pm.add(llvm::createPrintModulePass(dest2));

        pm.run(*llvm_module);
        dest.flush();
        dest2.flush();

    } else {
        // Just emit the assembly
        pm.run(*llvm_module);
        dest.flush();
    }

    debug("Assembly output succeeded");
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
    if (debugFlag) {
        scanner->debug(msg, true);
    }
}
void Parser::debugParseTrace(std::string msg) {
    if (debugFlag) {
        std::cout << "parse: " << msg << std::endl;
    }
}

/* Check if the current token is of type t
 * If it is, scan for the next token and store it
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


    // Code gen: main function
    std::vector<llvm::Type*> params;
    llvm::FunctionType *ft = llvm::FunctionType::get(llvm_builder->getInt32Ty(), params, false);
    llvm::Function *func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", *llvm_module);

    // By defining up here we guarantee this is the first "main" function declared, so no suffix in LLVM
    // which makes it the program entrypoint

    // Set global scope's procedure as main for easy access to LLVM
    Symbol s("main", T_IDENTIFIER, ST_PROCEDURE, TYPE_INT);
    s.llvm_function = func;
    scoper->setCurrentProcedure(s);


    if (!programBody()) {
        return false;
    }
    if (!isTokenType(T_PERIOD)) {
        warning("Missing \'.\' at end of program");
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
    debugParseTrace("Program Header");

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

    // Now that module is created, add runtime functions
    scoper->insertRuntimeFunctions(llvm_module, llvm_builder);

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
    debugParseTrace("Program Body");

    if (!declarationBlockHelper()) {
        return false;
    }
    if (!isTokenType(T_BEGIN)) {
        error("Missing \'begin\' keyword in program body");
        return false;
    }


    llvm::Function *func = scoper->getCurrentProcedure().llvm_function;

    // Code gen: Set main entrypoint
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(*llvm_context, "entry", func);
    llvm_builder->SetInsertPoint(entry);

    
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

    // Code gen: end main function, return 0
    llvm::Value *retVal = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, 0, true));
    llvm_builder->CreateRet(retVal);

    return true;
}

/* <declaration> ::=
 *      [ global ] <procedure_declaration>
 *    | [ global ] <variable_declaration>
 */
bool Parser::declaration() {
    debugParseTrace("Declaration");

    Symbol decl;
    decl.isGlobal = isTokenType(T_GLOBAL) ||
                    scoper->isCurrentScopeGlobal(); 
    // Outermost scope is still global even without keyword

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
    debugParseTrace("Procedure Declaration");

    if (!procedureHeader(decl)) {
        return false;
    }

    decl.symbolType = ST_PROCEDURE;

    // Error if duplicate name within function scope
    if (scoper->hasSymbol(decl.id, decl.isGlobal)) {
        error("Procedure name \'" + decl.id + "\' already used in this scope");
        return false;
    }


    // Code gen: function
    std::vector<llvm::Type*> params;
    llvm::Type *ty = nullptr;
    for (auto &p: decl.params) {
        ty = getLLVMType(p.type);
        if (p.isArr) {
            // []* array pointer type, since the arg passed is an address to array
            ty = llvm::ArrayType::get(ty, p.arrSize)->getPointerTo();
        }
        params.push_back(ty);
    }
    llvm::FunctionType *ft = llvm::FunctionType::get(getLLVMType(decl.type), params, false);
    llvm::Function *func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, decl.id, *llvm_module);
    // Set param names
    int i = 0;
    for (auto &p: func->args()) {
        p.setName(decl.params[i++].id);
    }
    decl.llvm_function = func;


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
    debugParseTrace("Procedure Header");

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
    debugParseTrace("Parameter List");

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
    debugParseTrace("Parameter");
    return variableDeclaration(param);
}

/* <procedure_body> ::=
 *          ( <declaration> ; )*
 *      begin
 *          ( <statement> ; )*
 *      end procedure
 */
bool Parser::procedureBody() {
    debugParseTrace("Procedure Body");

    if (!declarationBlockHelper()) {
        return false;
    }
    if (!isTokenType(T_BEGIN)) {
        error("Missing \'begin\' keyword in procedure body");
        return false;
    }

    Symbol currProc = scoper->getCurrentProcedure();
    llvm::Function *func = currProc.llvm_function;

    // Code gen: Set entrypoint
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(*llvm_context, "entry", func);
    llvm_builder->SetInsertPoint(entry);

    // Code gen: Allocate params and declared variables
    for (SymbolTable::iterator it = scoper->getScopeBegin();
         it != scoper->getScopeEnd();
         ++it) {

        if (it->second.symbolType != ST_VARIABLE) { continue; }

        llvm::Type *ty = getLLVMType(it->second.type);
        if (it->second.isArr) {
            ty = llvm::ArrayType::get(ty, it->second.arrSize);
        }

        llvm::Value *addr = llvm_builder->CreateAlloca(
            ty, 
            nullptr,
            it->second.id);

        it->second.llvm_address = addr;
    }

    // Code gen: Store argument values in allocated addresses
    auto arg = func->arg_begin();
    for (auto &p: currProc.params) {
        // Params vector is not up to date with llvm addresses
        // Get the symbol
        Symbol param = scoper->getSymbol(p.id);

        // Get the arg value and increment to next arg
        llvm::Value *argVal = arg++;

        // Store arg value in address
        if (param.isArr) {
            // Create a dummy symbol to pass
            // Type must be same as param,
            // otherwise it would've failed in compatibleTypeCheckCodeGen
            Symbol tempArgVal("", T_IDENTIFIER, ST_VARIABLE, param.type);
            tempArgVal.llvm_address = argVal;
            // argVal is an llvm_address to the array

            // Loop through each index and copy values from
            // argument to parameter (local) array
            arrayAssignmentCodeGen(param, tempArgVal);

        } else {
            // argVal is an llvm_value
            llvm_builder->CreateStore(argVal, param.llvm_address);

            // Update symbol
            param.llvm_value = argVal;
            scoper->setSymbol(param.id, param, param.isGlobal);
        }
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


    // Verify function. Must have a return value by now
    bool invalid = llvm::verifyFunction(*func, &llvm::errs());
    if (invalid) {
        error("Errors found in procedure");
        if (debugCodegenFlag) {
            llvm_module->print(llvm::outs(), nullptr);
        }
        return false;
    }
    return true;
}


/* <variable_declaration> ::=
 *      variable <identifier> : <type_mark> [ [ <bound> ] ]
 */
bool Parser::variableDeclaration(Symbol &decl) {
    debugParseTrace("Variable Declaration");

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

    // Code gen: Global variable allocation
    if (decl.isGlobal) {
        // Previously did this in bulk in program body,
        // but then global variables don't have an LLVM address when 
        // they may be accessed inside procedures

        llvm::Type *ty = getLLVMType(decl.type);
        if (decl.isArr) {
            ty = llvm::ArrayType::get(ty, decl.arrSize);
        }

        // Default initialized value
        llvm::Constant *initValue = llvm::Constant::getNullValue(ty);
        llvm::Value *addr = new llvm::GlobalVariable(
            *llvm_module,
            ty,
            false,
            llvm::GlobalValue::ExternalLinkage,
            initValue,
            decl.id);

        decl.llvm_address = addr;
    }

    // Set in scope
    scoper->setSymbol(decl.id, decl, decl.isGlobal);
    
    return true;
}


/* <type_mark> ::=
 *      integer | float | string | bool
 */
bool Parser::typeMark(Symbol &id) {
    debugParseTrace("Type Mark");

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
    debugParseTrace("Bound");

    Symbol num;
    int temp = token.getIntVal();

    if (number(num) && num.type == TYPE_INT && temp > 0) {
        id.arrSize = temp;
        return true;
    } else {
        error("Invalid bound. Must be a positive integer.");
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
    debugParseTrace("Statement");

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
    debugParseTrace("Assignment");

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
    if (!compatibleTypeCheckCodeGen(dest, exp)) {
        return false;
    }

    // Code gen: Assignment
    if (dest.isArr && !dest.isIndexed) {
        // Both dest and exp are unindexed arrays
        // Copy element by element
        arrayAssignmentCodeGen(dest, exp);

    } else {
        llvm_builder->CreateStore(exp.llvm_value, dest.llvm_address);
    }
    
    // Update symbol
    if (!dest.isArr) {
        dest.llvm_value = exp.llvm_value;
        scoper->setSymbol(dest.id, dest, dest.isGlobal);
    }
    return true;
}

/* <destination> ::= <identifier> [ [ <expression> ] ]
 */
bool Parser::destination(Symbol &id) {
    debugParseTrace("Destination");

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

    // Confirm that it is a name
    if (id.symbolType != ST_VARIABLE) {
        error("\'" + id.id + "\' is not a valid destination");
        return false;
    }

    Symbol ind;
    if (!arrayIndexHelper(id, ind)) {
        return false;
    }

    // Code gen: Get array destination address
    if (id.isIndexed) {
        llvm::Value *zeroVal = llvm::ConstantInt::get(
            *llvm_context,
            llvm::APInt(32, 0, true));
        
        // Get pointer to the element of the array
        id.llvm_address = llvm_builder->CreateInBoundsGEP(
            id.llvm_address,
            {zeroVal, ind.llvm_value});
    }
    return true;
}

/* <if_statement> ::=
 *      if ( <expression> ) then ( <statement> ; )*
 *      [ else ( <statement> ; )* ]
 *      end if
 */
bool Parser::ifStatement() {
    debugParseTrace("If");

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

    if (!isTokenType(T_RPAREN)) {
        error("Missing \')\' in if statement");
        return false;
    }

    // Type check/convert to bool
    if (exp.type == TYPE_INT) {
        exp.type = TYPE_BOOL;

        llvm::Value *zeroVal32 = llvm::ConstantInt::get(
            *llvm_context,
            llvm::APInt(32, 0, true));
        exp.llvm_value = llvm_builder->CreateICmpNE(
            exp.llvm_value, 
            zeroVal32);

    } else if (exp.type != TYPE_BOOL) {
        error("If statement expressions must evaluate to bool");
        return false;
    }

    // Code gen: If statement

    llvm::Function *func = scoper->getCurrentProcedure().llvm_function;

    llvm::Value *zeroVal = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(1, 0, true));
    llvm::Value *ifCond = llvm_builder->CreateICmpNE(
        exp.llvm_value, 
        zeroVal);
    exp.llvm_value = ifCond;

    llvm::BasicBlock *ifThenBB = llvm::BasicBlock::Create(*llvm_context, "ifThen", func);
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(*llvm_context, "ifElse", func);
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*llvm_context, "ifMerge", func);

    llvm_builder->CreateCondBr(ifCond, ifThenBB, elseBB);
    llvm_builder->SetInsertPoint(ifThenBB);


    if (!isTokenType(T_THEN)) {
        error("Missing \'then\' in if statement");
        return false;
    }
    if (!statementBlockHelper()) {
        return false;
    }

    // Merge ifThen block into merge if there wasn't a return
    if (llvm_builder->GetInsertBlock()->getTerminator() == nullptr) {
        llvm_builder->CreateBr(mergeBB);
    }

    llvm_builder->SetInsertPoint(elseBB);
    // Optional
    if (isTokenType(T_ELSE)) {
        if (!statementBlockHelper()) {
            return false;
        }
    }
    // Merge else block into merge if there wasn't a return
    if (llvm_builder->GetInsertBlock()->getTerminator() == nullptr) {
        llvm_builder->CreateBr(mergeBB);
    }

    llvm_builder->SetInsertPoint(mergeBB);

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
    debugParseTrace("Loop");

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


    // Code gen: Loop

    llvm::Function *func = scoper->getCurrentProcedure().llvm_function;

    llvm::BasicBlock *loopHeaderBB = llvm::BasicBlock::Create(*llvm_context, "loopHead", func);
    llvm::BasicBlock *loopBodyBB = llvm::BasicBlock::Create(*llvm_context, "loopBody", func);
    llvm::BasicBlock *loopMergeBB = llvm::BasicBlock::Create(*llvm_context, "loopMerge", func);

    llvm_builder->CreateBr(loopHeaderBB);
    llvm_builder->SetInsertPoint(loopHeaderBB);


    Symbol exp;
    if (!expression(exp)) {
        return false;
    }
    if (!isTokenType(T_RPAREN)) {
        error("Missing \')\' in loop");
        return false;
    }

    // Type check/convert to bool
    if (exp.type == TYPE_INT) {
        exp.type = TYPE_BOOL;

        llvm::Value *zeroVal32 = llvm::ConstantInt::get(
            *llvm_context,
            llvm::APInt(32, 0, true));
        exp.llvm_value = llvm_builder->CreateICmpNE(
            exp.llvm_value,
            zeroVal32);

    } else if (exp.type != TYPE_BOOL) {
        error("Loop statement expressions must evaluate to bool");
        return false;
    }

    // Code gen: Loop condition

    llvm::Value *zeroVal = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(1, 0, true));
    llvm::Value *loopCond = llvm_builder->CreateICmpNE(
        exp.llvm_value, 
        zeroVal);
    exp.llvm_value = loopCond;

    llvm_builder->CreateCondBr(loopCond, loopBodyBB, loopMergeBB);

    // Loop body
    llvm_builder->SetInsertPoint(loopBodyBB);

    if (!statementBlockHelper()) {
        return false;
    }

    // Go back to the header to check the condition
    // Merge else block into merge if there wasn't a return
    if (llvm_builder->GetInsertBlock()->getTerminator() == nullptr) {
        llvm_builder->CreateBr(loopHeaderBB);
    }

    llvm_builder->SetInsertPoint(loopMergeBB);

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
    debugParseTrace("Return");

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
    } else if (!compatibleTypeCheckCodeGen(proc, exp)) {
        return false;
    }

    // Code gen: Return
    llvm_builder->CreateRet(exp.llvm_value);
    return true;
}


/* <identifier> ::= [a-zA-Z][a-zA-Z0-9_]*
 */
bool Parser::identifier(Symbol &id) {
    debugParseTrace("Identifier");

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
    debugParseTrace("Expression");

    // Optional
    bool nt = isTokenType(T_NOT);

    if (!arithOp(exp)) {
        return false;
    }

    // Type check & code gen for not op
    if (nt) {
        if (exp.type == TYPE_BOOL || 
            exp.type == TYPE_INT) {
            exp.llvm_value = llvm_builder->CreateNot(exp.llvm_value);
        } else {
            error("\'not\' operator only defined for bool and int");
            return false;
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
    debugParseTrace("Expression Prime");

    Token op = token;
    if (isTokenType(T_AND) ||
        isTokenType(T_OR)) {
        Symbol rhs;
        if (!arithOp(rhs)) {
            return false;
        }

        // Check/convert type for & |
        expressionTypeCheckCodeGen(exp, rhs, op);

        if (!expression_p(exp)) {
            return false;
        }
    }
    return true;
}

/* <arithOp> ::= <relation> <arithOp_prime>
 */
bool Parser::arithOp(Symbol &arOp) {
    debugParseTrace("Arithmetic Op");

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
    debugParseTrace("Arithmetic Op Prime");

    Token op = token;
    if (isTokenType(T_PLUS) ||
        isTokenType(T_MINUS)) {
        Symbol rhs;
        if (!relation(rhs)) {
            return false;
        }

        // Check/convert type for + -
        if (!arithmeticTypeCheckCodeGen(arOp, rhs, op)) {
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
    debugParseTrace("Relation");

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
    debugParseTrace("Relation Prime");

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
        if (!relationTypeCheckCodeGen(rel, rhs, op)) {
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
    debugParseTrace("Term");

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
    debugParseTrace("Term Prime");

    Token op = token;
    if (isTokenType(T_MULTIPLY) ||
        isTokenType(T_DIVIDE)) {
        Symbol rhs;
        if (!factor(rhs)) {
            return false;
        }

        // Check/convert type for * /
        if (!arithmeticTypeCheckCodeGen(trm, rhs, op)) {
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
    debugParseTrace("Factor");

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
        if (name(fac) || number(fac)) {
            // Code gen: negative
            if (fac.type == TYPE_INT) {
                fac.llvm_value = llvm_builder->CreateNeg(fac.llvm_value);
            } else if (fac.type == TYPE_FLOAT) {
                fac.llvm_value = llvm_builder->CreateFNeg(fac.llvm_value);
            } else {
                error("Minus operator only valid on integers or floats");
                return false;
            }
        } else {
            error("Invalid use of minus operator");
            return false;
        }
    } else if (number(fac)) {
        
    } else if (string(fac)) {
        
    } else if (isTokenType(T_TRUE)) {
        fac.tokenType = T_TRUE;
        fac.type = TYPE_BOOL;
        fac.llvm_value = llvm::ConstantInt::getTrue(*llvm_context);

    } else if (isTokenType(T_FALSE)) {
        fac.tokenType = T_FALSE;
        fac.type = TYPE_BOOL;
        fac.llvm_value = llvm::ConstantInt::getFalse(*llvm_context);

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
    debugParseTrace("Proc Call or Name");

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
        debugParseTrace("Procedure Call");

        // Confirm that it is a procedure
        if (id.symbolType != ST_PROCEDURE) {
            error("\'" + id.id + "\' is not a procedure, and cannot be called");
            return false;
        }

        // Optional
        std::vector<llvm::Value*> argList;
        argumentList(id, argList);
        if (errorFlag) {
            return false;
        }

        if (!isTokenType(T_RPAREN)) {
            error("Missing \')\' in procedure call");
            return false;
        }


        // Code gen: Procedure call
        id.llvm_value = llvm_builder->CreateCall(id.llvm_function, argList);

    } else {
        debugParseTrace("Name");

        // Confirm that it is a name
        if (id.symbolType != ST_VARIABLE) {
            error("\'" + id.id + "\' is not a variable");
            return false;
        }

        // Check if array access
        Symbol ind;
        if (!arrayIndexHelper(id, ind)) {
            return false;
        }
        if (!nameCodeGen(id, ind)) {
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
    debugParseTrace("Name");

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

    // Confirm that it is a name
    if (id.symbolType != ST_VARIABLE) {
        error("\'" + id.id + "\' is not a variable");
        return false;
    }

    Symbol ind;
    if (!arrayIndexHelper(id, ind)) {
        return false;
    }
    if (!nameCodeGen(id, ind)) {
        return false;
    }
    return true;
}

/* Handle array access index
 * [ [ <expression> ] ]
 */
bool Parser::arrayIndexHelper(Symbol &id, Symbol &ind) {
    debugParseTrace("Index");

    // Optional
    if (isTokenType(T_LBRACKET)) {
        if (!expression(ind)) {
            return false;
        }

        // Check valid access
        if (!id.isArr) {
            error("\'" + id.id + "\' is not an array, and cannot be indexed");
            return false;
        } else if (ind.type != TYPE_INT) {
            error("Array index must be type integer");
            return false;
        }
        
        // Code gen: check 0 <= exp value < arr bound
        llvm::Value *zeroVal = llvm::ConstantInt::get(
            *llvm_context,
            llvm::APInt(32, 0, true));
        llvm::Value *boundVal = llvm::ConstantInt::get(
            *llvm_context,
            llvm::APInt(32, id.arrSize, true));
        llvm::Value *ltBound = llvm_builder->CreateICmpSLT(
            ind.llvm_value, 
            boundVal);
        llvm::Value *gteZero = llvm_builder->CreateICmpSGE(
            ind.llvm_value,
            zeroVal);
        llvm::Value *cond = llvm_builder->CreateAnd(ltBound, gteZero);
        
        llvm::Function *func = scoper->getCurrentProcedure().llvm_function;
        llvm::BasicBlock *boundErrBB = llvm::BasicBlock::Create(*llvm_context, "boundErr", func);
        llvm::BasicBlock *noErrBB = llvm::BasicBlock::Create(*llvm_context, "noErr", func);

        // If invalid index, display error and exit
        llvm_builder->CreateCondBr(cond, noErrBB, boundErrBB);
        llvm_builder->SetInsertPoint(boundErrBB);
        llvm::Function *errFunc = scoper->getSymbol("_outOfBoundsError").llvm_function;
        llvm_builder->CreateCall(errFunc, {});
        // Need a terminator to satisfy LLVM, but it will exit(1) before reaching
        llvm_builder->CreateBr(noErrBB);
        llvm_builder->SetInsertPoint(noErrBB);


        // Identifier is indexed
        id.isIndexed = true;

        if (!isTokenType(T_RBRACKET)) {
            error("Missing \']\' in name");
            return false;
        }
    }
    return true;
}

bool Parser::nameCodeGen(Symbol &id, Symbol &ind) {
    if (id.isArr) {
        if (!id.isIndexed) {
            // Either passing whole array as arg, or doing fancy array assignment
            return true;
        }

        llvm::Value *zeroVal = llvm::ConstantInt::get(
            *llvm_context,
            llvm::APInt(32, 0, true));
        
        // Get pointer to the element of the array
        id.llvm_address = llvm_builder->CreateInBoundsGEP(
            id.llvm_address,
            {zeroVal, ind.llvm_value});
    }
    id.llvm_value = llvm_builder->CreateLoad(getLLVMType(id.type), id.llvm_address);
    return true;
}

/* <argument_list> ::=
 *      <expression> , <argument_list>
 *    | <expression>
 */
bool Parser::argumentList(Symbol &id, std::vector<llvm::Value*> &argList) {
    debugParseTrace("Argument List");

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
    } else if (!compatibleTypeCheckCodeGen(id.params[argInd], arg)) {
        return false;
    }
    
    if (arg.isArr && !arg.isIndexed) {
        // Passing the entire array as arg
        // procedureBody will copy in the values to the local array
        argList.push_back(arg.llvm_address);
    } else {
        argList.push_back(arg.llvm_value);
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
        } else if (!compatibleTypeCheckCodeGen(id.params[argInd], arg)) {
            return false;
        }
        
        if (arg.isArr && !arg.isIndexed) {
            // Passing the entire array as arg
            // procedureBody will copy in the values to the local array
            argList.push_back(arg.llvm_address);
        } else {
            argList.push_back(arg.llvm_value);
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
    debugParseTrace("Number");

    // Check without consuming
    if (token.type == T_INTEGER_VAL) {
        num.type = TYPE_INT;
        num.tokenType = T_INTEGER_VAL;
        num.llvm_value = llvm::ConstantInt::get(
            *llvm_context,
            llvm::APInt(32, token.getIntVal(), true));
        
        // Consume token
        return isTokenType(T_INTEGER_VAL);

    //Check without consuming
    } else if (token.type == T_FLOAT_VAL) {
        num.type = TYPE_FLOAT;
        num.tokenType = T_FLOAT_VAL;
        num.llvm_value = llvm::ConstantFP::get(
            *llvm_context,
            llvm::APFloat(token.getFloatVal()));

        // Consume token
        return isTokenType(T_FLOAT_VAL);
    } else {
        return false;
    }
}

/* <string> :: = "[^"]*"
 */
bool Parser::string(Symbol &str) {
    debugParseTrace("String");

    // Check without consuming
    if (token.type == T_STRING_VAL) {
        str.id = token.val;
        str.tokenType = token.type;
        str.type = TYPE_STRING;
        str.llvm_value = llvm_builder->CreateGlobalStringPtr(token.val);
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
bool Parser::arithmeticTypeCheckCodeGen(Symbol &lhs, Symbol &rhs, Token &op) {
    debugParseTrace("Arithmetic type check code gen");
    if ((lhs.type != TYPE_INT && lhs.type != TYPE_FLOAT) ||
        (rhs.type != TYPE_INT && rhs.type != TYPE_FLOAT)) {
        error("Arithmetic only defined for int and float");
        return false;
    }
    
    if ((lhs.isArr && !lhs.isIndexed) ||
        (rhs.isArr && !rhs.isIndexed)) {
        // Unindexed arrays do op on whole array
        return arrayOpTypeCheckCodeGen(lhs, rhs, op);

    } else if (lhs.type == TYPE_INT) {
        if (rhs.type == TYPE_FLOAT) {
            // Convert lhs to float
            lhs.type = TYPE_FLOAT;
            lhs.llvm_value = llvm_builder->CreateSIToFP(
                lhs.llvm_value,
                llvm_builder->getFloatTy());
        }
        // Else both int, types match
        
    } else { // lhs is float
        if (rhs.type == TYPE_INT) {
            // Convert rhs to float
            rhs.type = TYPE_FLOAT;
            rhs.llvm_value = llvm_builder->CreateSIToFP(
                rhs.llvm_value,
                llvm_builder->getFloatTy());
        }
        // Else both float, types match
    }

    // Code gen
    switch (op.type) {
        case T_PLUS:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFAdd(lhs.llvm_value, rhs.llvm_value);
            } else {
                lhs.llvm_value = llvm_builder->CreateAdd(lhs.llvm_value, rhs.llvm_value);
            }
            break;
        case T_MINUS:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFSub(lhs.llvm_value, rhs.llvm_value);
            } else {
                lhs.llvm_value = llvm_builder->CreateSub(lhs.llvm_value, rhs.llvm_value);
            }
            break;
        case T_MULTIPLY:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFMul(lhs.llvm_value, rhs.llvm_value);
            } else {
                lhs.llvm_value = llvm_builder->CreateMul(lhs.llvm_value, rhs.llvm_value);
            }
            break;
        case T_DIVIDE:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFDiv(lhs.llvm_value, rhs.llvm_value);
            } else {
                lhs.llvm_value = llvm_builder->CreateSDiv(lhs.llvm_value, rhs.llvm_value);
            }
            break;
        default:
            error("Invalid arithmetic operator");
            return false;
    }
    return true;
}

/* Type checking for relational operators < <= > >= == !=
 */
bool Parser::relationTypeCheckCodeGen(Symbol &lhs, Symbol &rhs, Token &op) {
    debugParseTrace("Relation type check code gen");
    bool compatible = false;
    // If int is present with float or bool, convert int to that type
    // Otherwise types must match exactly

    // 0 used for comparisons below
    llvm::Value *zeroVal = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, 0, true));
    
    if ((lhs.isArr && !lhs.isIndexed) ||
        (rhs.isArr && !rhs.isIndexed)) {
        // Unindexed arrays do op on whole array
        return arrayOpTypeCheckCodeGen(lhs, rhs, op);

    } else if (lhs.type == TYPE_INT) {
        if (rhs.type == TYPE_BOOL) {
            compatible = true;
            // Convert lhs to bool
            lhs.type = TYPE_BOOL;
            // All non-zero values are true
            lhs.llvm_value = llvm_builder->CreateICmpNE(lhs.llvm_value, zeroVal);

        } else if (rhs.type == TYPE_FLOAT) {
            compatible = true;
            // Convert lhs to float
            lhs.type = TYPE_FLOAT;
            lhs.llvm_value = llvm_builder->CreateSIToFP(
                lhs.llvm_value,
                llvm_builder->getFloatTy());

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
            rhs.llvm_value = llvm_builder->CreateSIToFP(
                rhs.llvm_value,
                llvm_builder->getFloatTy());
        }

    } else if (lhs.type == TYPE_BOOL) {
        if (rhs.type == TYPE_BOOL) {
            compatible = true;

        } else if (rhs.type == TYPE_INT) {
            compatible = true;
            // Convert rhs to bool
            rhs.type = TYPE_BOOL;
            // All non-zero values are true
            rhs.llvm_value = llvm_builder->CreateICmpNE(rhs.llvm_value, zeroVal);
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
        return false;
    }


    // Code gen
    switch (op.type) {
        case T_LESS:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFCmpOLT(lhs.llvm_value, rhs.llvm_value);
            } else if (lhs.type == TYPE_BOOL) {
                lhs.llvm_value = llvm_builder->CreateICmpULT(lhs.llvm_value, rhs.llvm_value);
            } else { // Int
                lhs.llvm_value = llvm_builder->CreateICmpSLT(lhs.llvm_value, rhs.llvm_value);
            }
            break;
        case T_LESS_EQ:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFCmpOLE(lhs.llvm_value, rhs.llvm_value);
            } else if (lhs.type == TYPE_BOOL) {
                lhs.llvm_value = llvm_builder->CreateICmpULE(lhs.llvm_value, rhs.llvm_value);
            } else { // Int
                lhs.llvm_value = llvm_builder->CreateICmpSLE(lhs.llvm_value, rhs.llvm_value);
            }
            break;
        case T_GREATER:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFCmpOGT(lhs.llvm_value, rhs.llvm_value);
            } else if (lhs.type == TYPE_BOOL) {
                lhs.llvm_value = llvm_builder->CreateICmpUGT(lhs.llvm_value, rhs.llvm_value);
            } else { // Int
                lhs.llvm_value = llvm_builder->CreateICmpSGT(lhs.llvm_value, rhs.llvm_value);
            }
            break;
        case T_GREATER_EQ:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFCmpOGE(lhs.llvm_value, rhs.llvm_value);
            } else if (lhs.type == TYPE_BOOL) {
                lhs.llvm_value = llvm_builder->CreateICmpUGE(lhs.llvm_value, rhs.llvm_value);
            } else { // Int
                lhs.llvm_value = llvm_builder->CreateICmpSGE(lhs.llvm_value, rhs.llvm_value);
            }
            break;
        case T_EQUAL:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFCmpOEQ(lhs.llvm_value, rhs.llvm_value);
            } else if (lhs.type == TYPE_STRING) {
                lhs.llvm_value = stringEqualHelper(lhs, rhs);
            } else { // Int or Bool
                lhs.llvm_value = llvm_builder->CreateICmpEQ(lhs.llvm_value, rhs.llvm_value);
            }
            break;
        case T_NOT_EQUAL:
            if (lhs.type == TYPE_FLOAT) {
                lhs.llvm_value = llvm_builder->CreateFCmpONE(lhs.llvm_value, rhs.llvm_value);
            } else if (lhs.type == TYPE_STRING) {
                lhs.llvm_value = llvm_builder->CreateNot(stringEqualHelper(lhs, rhs));
            } else { // Int or Bool
                lhs.llvm_value = llvm_builder->CreateICmpNE(lhs.llvm_value, rhs.llvm_value);
            }
            break; 
        default:
            error("Invalid relational operator");
            return false;
    }
    return compatible;
}


/* Compare strings character by character. Return LLVM value for whether they are equal.
 */
llvm::Value* Parser::stringEqualHelper(Symbol &lhs, Symbol &rhs) {
    debugParseTrace("String equality");
    llvm::Function *func = scoper->getCurrentProcedure().llvm_function;

    llvm::BasicBlock *strCmpBB = llvm::BasicBlock::Create(*llvm_context, "strCmp", func);
    llvm::BasicBlock *strCmpMergeBB = llvm::BasicBlock::Create(*llvm_context, "strCmpMerge", func);

    // Initial index = 0
    llvm::Value *indAddr = llvm_builder->CreateAlloca(
        getLLVMType(TYPE_INT), 
        nullptr,
        "strCmpInd");
    llvm::Value *index = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, 0, true));
    llvm_builder->CreateStore(index, indAddr);

    llvm_builder->CreateBr(strCmpBB);
    llvm_builder->SetInsertPoint(strCmpBB);

    index = llvm_builder->CreateLoad(getLLVMType(TYPE_INT), indAddr);


    // Get element pointer to string character, then load the character
    llvm::Value *lhsCharAddr = llvm_builder->CreateInBoundsGEP(
        lhs.llvm_value,
        index);
    llvm::Value *rhsCharAddr = llvm_builder->CreateInBoundsGEP(
        rhs.llvm_value,
        index);
    llvm::Value *lhsCharVal = llvm_builder->CreateLoad(
        llvm_builder->getInt8Ty(), 
        lhsCharAddr);
    llvm::Value *rhsCharVal = llvm_builder->CreateLoad(
        llvm_builder->getInt8Ty(), 
        rhsCharAddr);

    // lhs == rhs ?
    llvm::Value *cmp = llvm_builder->CreateICmpEQ(lhsCharVal, rhsCharVal);


    // Null terminator char \0
    llvm::Value *zeroVal8 = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(8, 0, true));
    
    // See if one char is null terminator. 
    // Ignore the rhs char since if they're unequal it doesn't matter anyway
    llvm::Value *notNullTerm = llvm_builder->CreateICmpNE(lhsCharVal, zeroVal8);

    // Increment index
    llvm::Value *increment = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, 1, true));
    index = llvm_builder->CreateAdd(index, increment);
    llvm_builder->CreateStore(index, indAddr);


    // Keep checking if not the end AND lhs == rhs so far
    llvm::Value *andCond = llvm_builder->CreateAnd(cmp, notNullTerm);
    llvm_builder->CreateCondBr(andCond, strCmpBB, strCmpMergeBB);

    llvm_builder->SetInsertPoint(strCmpMergeBB);
    return cmp;
}


/* Type checking for expression operators & |
 */
bool Parser::expressionTypeCheckCodeGen(Symbol &lhs, Symbol &rhs, Token &op) {
    debugParseTrace("Expression type check code gen");
    bool compatible = false;
    
    if (lhs.type == TYPE_BOOL && rhs.type == TYPE_BOOL) {
        compatible = true;
    } else if (lhs.type == TYPE_INT && rhs.type == TYPE_INT) {
        compatible = true;
    }

    if (!compatible) {
        error("Expression ops only defined for bool and int");
        return false;
    }

    if ((lhs.isArr && !lhs.isIndexed) ||
        (rhs.isArr && !rhs.isIndexed)) {
        // Unindexed arrays do op on whole array
        return arrayOpTypeCheckCodeGen(lhs, rhs, op);
    }

    // Code gen
    switch (op.type) {
        case T_AND:
            lhs.llvm_value = llvm_builder->CreateAnd(lhs.llvm_value, rhs.llvm_value);
            break;
        case T_OR:
            lhs.llvm_value = llvm_builder->CreateOr(lhs.llvm_value, rhs.llvm_value);
            break;
        default:
            error("Invalid expression operator");
            return false;
    }
    return compatible;
}


/* Type checking for assignment operator =
 * and matching params to arguments
 */
bool Parser::compatibleTypeCheckCodeGen(Symbol &dest, Symbol &exp) {
    debugParseTrace("Compatible type check");
    bool compatible = false;
    
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
                error("Both arrays must be either indexed or unindexed");
                return false;
            } else if (!dest.isIndexed) {
                // Both are unindexed. Array lengths must match
                if (dest.arrSize != exp.arrSize) {
                    error("Array lengths must match");
                    return false;
                }
            } else { // Else both are indexed
                // Types must match exactly
                if (dest.type != exp.type) {
                    error("Unindexed array types must match exactly");
                    return false;
                }
            }
        } else {
            // One side is array
            // Array must be indexed
            if ((dest.isArr && !dest.isIndexed) ||
                (exp.isArr && !exp.isIndexed)) {
                error("Array is not indexed");
                return false;
            }
        }
    }
    // Else both are non-array


    // 0 used for comparisons below
    llvm::Value *zeroVal = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, 0, true));


    /* If types are compatible, convert exp to type of dest
     * int <-> bool
     * int <-> float
     * Otherwise types must match exactly
     * 
     * If working with unindexed arrays, don't do the type conversion
     * but still want to check that types are compatible
     */
    if (dest.type == exp.type) {
        compatible = true;
    } else if (dest.type == TYPE_INT) {
        if (exp.type == TYPE_BOOL) {
            compatible = true;

            if (!(exp.isArr && !exp.isIndexed)) {
                // Convert exp to int
                exp.type = TYPE_INT;
                exp.llvm_value = llvm_builder->CreateIntCast(
                    exp.llvm_value,
                    llvm_builder->getInt32Ty(),
                    false);
            }
        } else if (exp.type == TYPE_FLOAT) {
            compatible = true;

            if (!(exp.isArr && !exp.isIndexed)) {
                // Convert exp to int
                exp.type = TYPE_INT;
                exp.llvm_value = llvm_builder->CreateFPToSI(
                    exp.llvm_value,
                    llvm_builder->getInt32Ty());
            }
        }
    } else if (dest.type == TYPE_FLOAT) {
        if (exp.type == TYPE_INT) {
            compatible = true;

            if (!(exp.isArr && !exp.isIndexed)) {
                // Convert exp to float
                exp.type = TYPE_FLOAT;
                exp.llvm_value = llvm_builder->CreateSIToFP(
                    exp.llvm_value,
                    llvm_builder->getFloatTy());
            }
        }
    } else if (dest.type == TYPE_BOOL) {
        if (exp.type == TYPE_INT) {
            compatible = true;

            if (!(exp.isArr && !exp.isIndexed)) {
                // Convert exp to bool
                exp.type = TYPE_BOOL;
                exp.llvm_value = llvm_builder->CreateICmpNE(
                    exp.llvm_value, 
                    zeroVal);
            }
        }
    }

    if (!compatible) {
        error("Incompatible types " + getTypeName(dest.type) + " and " + getTypeName(exp.type));
    }
    return compatible;
}


/* Ops done on unindexed arrays affect the whole array
 */
bool Parser::arrayOpTypeCheckCodeGen(Symbol &lhs, Symbol &rhs, Token &op) {
    debugParseTrace("Array type check code gen");
    // If both arrays, must be same size
    if (lhs.isArr && !lhs.isIndexed && rhs.isArr && !rhs.isIndexed &&
        lhs.arrSize != rhs.arrSize) {
        error("Operations with two unindexed arrays must have arrays of same size");
        return false;
    }

    // Get the correct type for the array
    // No need to check every type matching here.
    // Error will be thrown from TypeCheckCodeGen funcs if invalid matches
    llvm::Type *ty = nullptr;
    Type outputType;
    switch (op.type) {
        case T_PLUS:
        case T_MINUS:
        case T_MULTIPLY:
        case T_DIVIDE:
            if (lhs.type == TYPE_FLOAT || rhs.type == TYPE_FLOAT) {
                outputType = TYPE_FLOAT;
                ty = getLLVMType(TYPE_FLOAT);
            } else {
                outputType = TYPE_INT;
                ty = getLLVMType(TYPE_INT);
            }
            break;
        case T_LESS:
        case T_LESS_EQ:
        case T_GREATER:
        case T_GREATER_EQ:
        case T_EQUAL:
        case T_NOT_EQUAL:
            outputType = TYPE_BOOL;
            ty = getLLVMType(TYPE_BOOL);
            break;
        case T_AND:
        case T_OR:
            if (lhs.type == TYPE_BOOL) {
                outputType = TYPE_BOOL;
                ty = getLLVMType(TYPE_BOOL);
            } else {
                outputType = TYPE_INT;
                ty = getLLVMType(TYPE_INT);
            }
            break;
        default:
            error("Invalid unindexed array operator");
            return false;
    }

    int arrSize = lhs.arrSize;
    if (!(lhs.isArr && !lhs.isIndexed)) {
        arrSize = rhs.arrSize;
    }
    ty = llvm::ArrayType::get(ty, arrSize);

    // Allocate a new array to store the result
    llvm::Value *resultArrAddr = llvm_builder->CreateAlloca(
        ty, 
        nullptr,
        "");


    llvm::Function *func = scoper->getCurrentProcedure().llvm_function;
    llvm::BasicBlock *arrOpBB = llvm::BasicBlock::Create(*llvm_context, "arrOp", func);
    llvm::BasicBlock *arrOpMergeBB = llvm::BasicBlock::Create(*llvm_context, "arrOpMerge", func);

    // Initial index = 0
    llvm::Value *indAddr = llvm_builder->CreateAlloca(
        getLLVMType(TYPE_INT), 
        nullptr,
        "arrOpInd");
    llvm::Value *zeroVal = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, 0, true));
    llvm::Value *index = zeroVal;
    llvm_builder->CreateStore(index, indAddr);

    // Max value of index is arrSize - 1
    llvm::Value *loopEnd = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, arrSize, true));

    llvm_builder->CreateBr(arrOpBB);
    llvm_builder->SetInsertPoint(arrOpBB);

    index = llvm_builder->CreateLoad(getLLVMType(TYPE_INT), indAddr);


    // If the operand is an unindexed array, load the element of the current index
    Symbol lhsElem("", lhs.tokenType, lhs.symbolType, lhs.type);
    if (lhs.isArr && !lhs.isIndexed) {
        // Get pointer to array element, and load the value
        llvm::Value *elemAddr = llvm_builder->CreateInBoundsGEP(
            lhs.llvm_address,
            {zeroVal, index});
        lhsElem.llvm_value = llvm_builder->CreateLoad(
            getLLVMType(lhs.type), 
            elemAddr);
    } else {
        lhsElem.llvm_value = lhs.llvm_value;
    }

    Symbol rhsElem("", rhs.tokenType, rhs.symbolType, rhs.type);
    if (rhs.isArr && !rhs.isIndexed) {
        // Get pointer to array element, and load the value
        llvm::Value *elemAddr = llvm_builder->CreateInBoundsGEP(
            rhs.llvm_address,
            {zeroVal, index});
        rhsElem.llvm_value = llvm_builder->CreateLoad(
            getLLVMType(rhs.type), 
            elemAddr);
    } else {
        rhsElem.llvm_value = rhs.llvm_value;
    }

    switch (op.type) {
        case T_PLUS:
        case T_MINUS:
        case T_MULTIPLY:
        case T_DIVIDE:
            if (!arithmeticTypeCheckCodeGen(lhsElem, rhsElem, op)) {
                return false;
            }
            break;
        case T_LESS:
        case T_LESS_EQ:
        case T_GREATER:
        case T_GREATER_EQ:
        case T_EQUAL:
        case T_NOT_EQUAL:
            if (!relationTypeCheckCodeGen(lhsElem, rhsElem, op)) {
                return false;
            }
            break;
        case T_AND:
        case T_OR:
            if (!expressionTypeCheckCodeGen(lhsElem, rhsElem, op)) {
                return false;
            }
            break;
        default:
            error("Invalid unindexed array operator");
            return false;
    }

    // Get pointer to result array element, and store the result of the calculation
    llvm::Value *elemAddr = llvm_builder->CreateInBoundsGEP(
        resultArrAddr,
        {zeroVal, index});
    llvm_builder->CreateStore(lhsElem.llvm_value, elemAddr);


    // Increment index
    llvm::Value *increment = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, 1, true));
    index = llvm_builder->CreateAdd(index, increment);
    llvm_builder->CreateStore(index, indAddr);
    
    // index < arr size
    llvm::Value *cond = llvm_builder->CreateICmpSLT(index, loopEnd);
    llvm_builder->CreateCondBr(cond, arrOpBB, arrOpMergeBB);

    llvm_builder->SetInsertPoint(arrOpMergeBB);

    // Update the result symbol that will be passed up the tree
    lhs.llvm_address = resultArrAddr;
    lhs.isArr = true;
    lhs.isIndexed = false;
    lhs.arrSize = arrSize;
    lhs.type = outputType;

    return true;
}


/* Code gen to copy the elements from one array to another
 * Used for assignment and copying args to local scope
 */
void Parser::arrayAssignmentCodeGen(Symbol &dest, Symbol &exp) {
    debugParseTrace("Array assignment/copying");

    llvm::Function *func = scoper->getCurrentProcedure().llvm_function;

    llvm::BasicBlock *arrCopyBB = llvm::BasicBlock::Create(*llvm_context, "arrCopy", func);
    llvm::BasicBlock *arrCopyMergeBB = llvm::BasicBlock::Create(*llvm_context, "arrCopyMerge", func);

    // Initial index = 0
    llvm::Value *indAddr = llvm_builder->CreateAlloca(
        getLLVMType(TYPE_INT),
        nullptr,
        "arrCopyInd");
    llvm::Value *zeroVal = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, 0, true));
    llvm::Value *index = zeroVal;
    llvm_builder->CreateStore(index, indAddr);

    // Max value of index is arrSize - 1
    llvm::Value *loopEnd = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, dest.arrSize, true));

    llvm_builder->CreateBr(arrCopyBB);
    llvm_builder->SetInsertPoint(arrCopyBB);

    index = llvm_builder->CreateLoad(getLLVMType(TYPE_INT), indAddr);


    // Get pointer to array element, and load the value
    llvm::Value *expElemAddr = llvm_builder->CreateInBoundsGEP(
        exp.llvm_address,
        {zeroVal, index});
    llvm::Value *expElemVal = llvm_builder->CreateLoad(
        getLLVMType(dest.type), 
        expElemAddr);

    // Get pointer to dest array element, and store the value
    llvm::Value *destElemAddr = llvm_builder->CreateInBoundsGEP(
        dest.llvm_address,
        {zeroVal, index});
    llvm_builder->CreateStore(expElemVal, destElemAddr);
    

    // Increment index
    llvm::Value *increment = llvm::ConstantInt::get(
        *llvm_context,
        llvm::APInt(32, 1, true));
    index = llvm_builder->CreateAdd(index, increment);
    llvm_builder->CreateStore(index, indAddr);
    
    // index < arr size
    llvm::Value *cond = llvm_builder->CreateICmpSLT(index, loopEnd);
    llvm_builder->CreateCondBr(cond, arrCopyBB, arrCopyMergeBB);

    llvm_builder->SetInsertPoint(arrCopyMergeBB);
}


llvm::Type *Parser::getLLVMType(Type t) {
    switch (t) {
        case TYPE_INT:
            return llvm_builder->getInt32Ty();
        case TYPE_FLOAT:
            return llvm_builder->getFloatTy();
        case TYPE_STRING:
            // String = array of 8 bit ints
            return llvm_builder->getInt8PtrTy();
        case TYPE_BOOL:
            return llvm_builder->getInt1Ty();
        default:
            error("Invalid type");
            return nullptr;
    }
}