//
// Created by Kacper Raczy on 2019-09-13.
//

#ifndef frontend_semantic_analysis_hpp
#define frontend_semantic_analysis_hpp

extern "C" {
#define operator op
#include "ast/ast.h"
#undef operator
};

#include <iostream>
#include <unordered_set>
#include <vector>
#include <exception>
#include <string>
#include "SymbolTable.hpp"
#include "Record.hpp"

namespace semanticanalysis {

struct SemanticAnalysisException: public std::exception {
    std::string cause;
    explicit SemanticAnalysisException(std::string cause);
    const char* what() const throw() override;
};

struct SymbolException: public SemanticAnalysisException {
    using SemanticAnalysisException::SemanticAnalysisException;
    explicit SymbolException(std::string cause);
};

struct DeclarationException: public SemanticAnalysisException {
    using SemanticAnalysisException::SemanticAnalysisException;
    explicit DeclarationException(std::string cause);
};


/**
 * Performs following checks on declarations list:
 * - duplicates
 * - invalid indicies in array declarations
 * @param decList declaration list
 * @throws DeclarationException
 */
void analyze(ASTDeclarationList* decList);

/**
 * Checks if symbol match with declaration
 * @param symbol
 * @param table
 */
void validate(ASTSymbol symbol, SymbolTable* symbolTable);

/**
 * Performs following checks on assignments:
 * - operand symbol validation
 * - iterator mutation
 * @param assignment
 * @param symbolTable
 */
void analyze(ASTAssignment* assignment, SymbolTable* symbolTable);

/**
 * Performs following checks on conditions:
 * - operand symbol validation
 * @param condition
 * @param symbolTable
 */
void analyze(ASTCondition condition, SymbolTable* symbolTable);

/**
 * Performs following checks on io operation:
 * - constant used with READ
 * - operand symbol validation
 * @param io
 * @param symbolTable
 */
void analyze(ASTIO* io, SymbolTable* symbolTable);

/**
 * Performs following checks on branch operation
 * - condition validation
 * @param branch
 * @param symbolTable
 */
void analyze(ASTBranch* branch, SymbolTable* symbolTable);

/**
 * Performs following checks on while loop operation
 * - condition validation
 * @param whileLoop
 * @param symbolTable
 */
void analyze(ASTWhileLoop* whileLoop, SymbolTable* symbolTable);

/**
 * Performs following checks on for loop operation
 * - duplicate iterator symbol
 * - operand symbol validation
 * @param forLoop
 * @param symbolTable
 */
void analyze(ASTForLoop *forLoop, SymbolTable* symbolTable);

/**
 * Performs recursive semantic analysis on ASTree. Rules:
 * Type checks:
 * - int cannot be indexed
 * - array must be indexed
 * - array bounds in form (l:u) must fulfill l <= u
 * Identifier checks:
 * - identifier must be in symbol table, except for iterators
 * - identifier must be initialized, before use
 * Iterator checks:
 * - iterator is immutable (cannot be lvalue in assignment or operand in read)
 * @param node ASTNode to analyze
 * @param symbolTable valid symbol table
 */
void analyze(ASTNode* node, SymbolTable* symbolTable);

} /* semanticanalysis */

#endif /* frontend_semantic_analysis_hpp */
