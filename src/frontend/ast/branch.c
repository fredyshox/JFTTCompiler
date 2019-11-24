#include "branch.h"

ASTBranch* ASTBranchCreateWithElse(ASTCondition condition, ASTNode* ifnode, ASTNode* elsenode) {
  ASTBranch* branch = malloc(sizeof(ASTBranch));
  branch->condition = condition;
  branch->ifNode = ifnode;
  branch->elseNode = elsenode;

  return branch;
}

ASTBranch* ASTBranchCreate(ASTCondition condition, ASTNode* ifnode) {
  return ASTBranchCreateWithElse(condition, ifnode, NULL);
}
