//
// Created by Kacper Raczy on 2019-09-09.
//

#ifndef frontend_converter_hpp
#define frontend_converter_hpp

#include "blocks/ControlFlowBlock.hpp"
#include "blocks/WhileLoopBlock.hpp"
#include "blocks/ForLoopBlock.hpp"
#include "blocks/ConditionBlock.hpp"
#include "blocks/ThreeAddressCodeBlock.hpp"
#include "SymbolTable.hpp"

extern "C" {
#define operator op
#include "ast/assignment.h"
#include "ast/io.h"
#include "ast/expression.h"
#include "ast/loops.h"
#include "ast/branch.h"
#include "ast/program.h"
#undef operator
};

namespace irconverter {
    // blocks
    BaseBlock* convertList(ASTNode* node, SymbolTable* parentTable);
    BaseBlock* convert(ASTNode node, SymbolTable* parentTable);
    ForLoopBlock* convert(ASTForLoop forLoop, SymbolTable* parentTable);
    WhileLoopBlock* convert(ASTWhileLoop whileLoop, SymbolTable* parentTable);
    ConditionBlock* convert(ASTBranch branch, SymbolTable* parentTable);
    ThreeAddressCodeBlock* convert(ASTIO io);
    ThreeAddressCodeBlock* convert(ASTAssignment assignment);
    Condition convert(ASTCondition astCondition);
    std::unique_ptr<Operand> convert(ASTOperand astOperand);
    std::unique_ptr<Operand> convert(ASTSymbol symbol);
    std::unique_ptr<Operand> convert(ASTIndex astIndex);
    ThreeAddressCode::Operator convert(int op);
    // symbol table
    GlobalSymbolTable* symbolTableFrom(ASTDeclarationList* decList);
} /* irconverter */

#endif /* frontend_converter_hpp */
