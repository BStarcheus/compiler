#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "llvm/IR/LLVMContext.h"

class Parser {
    public:
        Parser(Scanner* scannerPtr, ScopeManager* scoperPtr, bool dbgParser, bool dbgCodegen);
        ~Parser();
        bool debugFlag;
        bool debugCodegenFlag;
        bool errorFlag;
        bool parse();
        bool outputAssembly();
    private:
        Token token;
        Scanner* scanner;
        ScopeManager* scoper;

        void error(std::string msg);
        void warning(std::string msg);
        void debug(std::string msg);
        void debugParseTrace(std::string msg);

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
        bool assignmentStatement();
        bool destination(Symbol &id);
        bool ifStatement();
        bool loopStatement();
        bool returnStatement();

        bool identifier(Symbol &id);

        bool expression(Symbol &exp);
        bool expression_p(Symbol &exp);
        bool arithOp(Symbol &arOp);
        bool arithOp_p(Symbol &arOp);
        bool relation(Symbol &rel);
        bool relation_p(Symbol &rel);
        bool term(Symbol &trm);
        bool term_p(Symbol &trm);
        bool factor(Symbol &fac);

        bool procCallOrName(Symbol &id);
        bool name(Symbol &id);
        bool arrayIndexHelper(Symbol &id, Symbol &ind);
        bool nameCodeGen(Symbol &id, Symbol &ind);
        bool argumentList(Symbol &id, std::vector<llvm::Value*> &argList);
        bool number(Symbol &num);
        bool string(Symbol &str);

        bool declarationBlockHelper();
        bool statementBlockHelper();

        bool arithmeticTypeCheckCodeGen(Symbol &lhs, Symbol &rhs, Token &op);
        bool relationTypeCheckCodeGen(Symbol &lhs, Symbol &rhs, Token &op);
        llvm::Value* stringEqualHelper(Symbol &lhs, Symbol &rhs);
        bool expressionTypeCheckCodeGen(Symbol &lhs, Symbol &rhs, Token &op);
        bool compatibleTypeCheck(Symbol &dest, Symbol &exp);

        // LLVM
        llvm::LLVMContext *llvm_context;
        llvm::Module *llvm_module;
        llvm::IRBuilder<> *llvm_builder;

        llvm::Type *getLLVMType(Type t);
};
#endif