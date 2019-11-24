#ifndef astassignment_h
#define astassignment_h

#include <stdlib.h>
#include "ast/types.h"
#include "ast/operand.h"
#include "ast/expression.h"

#define kRTypeExpression 0
#define kRTypeOperand 1

struct ASTAssignment {
  ASTSymbol symbol;
  int rtype;
  union {
    struct ASTOperand operand;
    struct ASTExpression expression;
  };
};

typedef struct ASTAssignment ASTAssignment;

ASTAssignment* ASTAssignmentCreateWithExpression(ASTSymbol symbol, ASTExpression expression);

ASTAssignment* ASTAssignmentCreateWithOperand(ASTSymbol symbol, ASTOperand operand);

void ASTAssignmentToString(ASTAssignment* assignment, char* buffer);

#endif /* astassignment_h */
