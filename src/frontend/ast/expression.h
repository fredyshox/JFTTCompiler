#ifndef astexpression_h
#define astexpression_h

#include "ast/types.h"
#include "ast/operand.h"

#define kOperatorAdd 0
#define kOperatorSub 1
#define kOperatorMul 2
#define kOperatorDiv 3
#define kOperatorMod 4

struct ASTExpression {
  struct ASTOperand operand1;
  struct ASTOperand operand2;
  int operator;
};

typedef struct ASTExpression ASTExpression;

ASTExpression ASTExpressionCreate(ASTOperand op1, ASTOperand op2, int operator);
void ASTExpressionToString(ASTExpression expr, char* buffer);

#endif /* astexpression_h */
