#ifndef astloops_h
#define astloops_h

#include <stdlib.h>
#include "condition.h"
#include "node.h"
#include "operand.h"
#include "types.h"

#define kLoopWhile 0
#define kLoopDoWhile 1
#define kLoopForTo 2
#define kLoopForDownTo 3

struct ASTWhileLoop {
  int type;
  ASTCondition condition;
  ASTNode* start;
};

typedef struct ASTWhileLoop ASTWhileLoop;

ASTWhileLoop* ASTLoopCreateWhile(ASTCondition condition, ASTNode* node);
ASTWhileLoop* ASTLoopCreateDoWhile(ASTCondition condition, ASTNode* node);

struct ASTForLoop {
  int type;
  ASTIdentifier iterator;
  ASTOperand iteratorInitial;
  ASTOperand iteratorBound;
  ASTNode* start;
};

typedef struct ASTForLoop ASTForLoop;

ASTForLoop* ASTLoopCreateForTo(ASTIdentifier it, ASTOperand initial, ASTOperand bound, ASTNode* start);
ASTForLoop* ASTLoopCreateForDownTo(ASTIdentifier it, ASTOperand initial, ASTOperand bound, ASTNode* start);
void ASTForLoopToString(ASTForLoop* forLoop, char* buffer);

#endif /* astloops_h */
