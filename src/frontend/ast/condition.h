#ifndef astcondition_h
#define astcondition_h

#include "ast/operand.h"

#define kCondOperatorEqual 0
#define kCondOperatorNotEqual 1
#define kCondOperatorGreater 2
#define kCondOperatorLess 3
#define kCondOperatorGreaterEqual 4
#define kCondOperatorLessEqual 5

struct ASTCondition {
  struct ASTOperand operand1;
  struct ASTOperand operand2;
  int operator;
};

typedef struct ASTCondition ASTCondition;

ASTCondition ASTConditionCreate(ASTOperand op1, ASTOperand op2, int operator);
void ASTConditionToString(ASTCondition condition, char* buffer);

#endif /* astcondition_h */
