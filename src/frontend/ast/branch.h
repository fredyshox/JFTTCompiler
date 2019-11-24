
#ifndef astbranch_h
#define astbranch_h

#include <stdlib.h>
#include "condition.h"
#include "node.h"

struct ASTBranch {
  ASTCondition condition;
  ASTNode* ifNode;
  ASTNode* elseNode;
};

typedef struct ASTBranch ASTBranch;

ASTBranch* ASTBranchCreateWithElse(ASTCondition condition, ASTNode* ifnode, ASTNode* elsenode);
ASTBranch* ASTBranchCreate(ASTCondition condition, ASTNode* ifnode);

#endif /* astbranch_h */
