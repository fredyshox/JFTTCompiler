#include "loops.h"

ASTWhileLoop* ASTLoopCreateWhile(ASTCondition condition, ASTNode* node) {
  ASTWhileLoop* loop = malloc(sizeof(ASTWhileLoop));
  loop->type = kLoopWhile;
  loop->condition = condition;
  loop->start = node;

  return loop;
}

ASTWhileLoop* ASTLoopCreateDoWhile(ASTCondition condition, ASTNode* node) {
  ASTWhileLoop* loop = malloc(sizeof(ASTWhileLoop));
  loop->type = kLoopDoWhile;
  loop->condition = condition;
  loop->start = node;

  return loop;
}

ASTForLoop* ASTLoopCreateForTo(ASTIdentifier it, ASTOperand initial, ASTOperand bound, ASTNode* start) {
  ASTForLoop* loop = malloc(sizeof(ASTForLoop));
  loop->type = kLoopForTo;
  loop->iterator = it;
  loop->iteratorInitial = initial;
  loop->iteratorBound = bound;
  loop->start = start;

  return loop;
}

ASTForLoop* ASTLoopCreateForDownTo(ASTIdentifier it, ASTOperand initial, ASTOperand bound, ASTNode* start) {
  ASTForLoop* loop = malloc(sizeof(ASTForLoop));
  loop->type = kLoopForDownTo;
  loop->iterator = it;
  loop->iteratorInitial = initial;
  loop->iteratorBound = bound;
  loop->start = start;

  return loop;
}

void ASTForLoopToString(ASTForLoop* forLoop, char* buffer) {
  int step = (forLoop->type == kLoopForTo) ? 1 : -1;
  char ostr1[OPERAND_MAX_STRLEN], ostr2[OPERAND_MAX_STRLEN];
  ASTOperandToString(forLoop->iteratorInitial, ostr1);
  ASTOperandToString(forLoop->iteratorBound, ostr2);
  sprintf(buffer, "FOR %s IN %s:%s:%d", forLoop->iterator, ostr1, ostr2, step);
}
