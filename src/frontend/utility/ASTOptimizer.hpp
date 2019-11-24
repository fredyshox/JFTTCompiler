//
// Created by Kacper Raczy on 2019-09-18.
//

#ifndef frontend_ast_optimizer_hpp
#define frontend_ast_optimizer_hpp

extern "C" {
#define operator op
#include "ast/ast.h"
#undef operator
};

#include <iostream>

namespace astoptimizer {

#define OPTIMIZER_BODY_REMOVED 0xe
#define OPTIMIZER_NEXT_NEW_START 0xf
// actions
#define ACTION_NOTHING 0
#define ACTION_EDIT 1
#define ACTION_EXTRACT_BODY1 2
#define ACTION_EXTRACT_BODY2 3
#define ACTION_REMOVE 4
// conditons
#define CONDITION_OK 0
#define CONDITION_ATRUE 1
#define CONDITION_AFALSE 2

/**
 * Available optimizations on ASTree
 *
 * Assignments:
 * - 2 constant operands to 1 operand
 * - remove trivial assignments (ex. a := a)
 *
 * Conditions:
 * - remove never-go branches
 * - trivial constant-only conditions
 *
 * For Loops:
 * - <=0 iteration loop ranges
 * @param program
 */
void optimize(ASTProgram* program);
int optimize(ASTNode* node);

/**
 * Optimizes operations based on following conditions:
 * - trivial assignment (ex. a := a)
 * - expression assignment to operand assignment when 2 operands are constants
 * @param assignment
 * @return action flag
 */
int optimize(ASTAssignment* assignment);

/**
 * Evaluates whether number of iterations is >0
 * @param forLoop
 * @return action flag
 */
int optimize(ASTForLoop* forLoop);

/**
 * Removal based on condition
 * @param loop
 * @return action flag
 */
int optimize(ASTWhileLoop* loop);

/**
 * Removal based on condition
 * @param branch
 * @return action flag
 */
int optimize(ASTBranch* branch);

/**
 * Evaluates whether condition is:
 * - always true
 * - always false
 * @param condition
 * @return condition flag
 */
int evaluateCondition(ASTCondition condition);

} /* astoptimizer */

#endif /* frontend_ast_optimizer_hpp */
