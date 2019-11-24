#ifndef astoperand_h
#define astoperand_h

#include "ast/types.h"
#include "ast/program.h"
#include <stdbool.h>

#define kOperandConstant 0
#define kOperandSymbol 1

struct ASTOperand {
  int type;
  union {
    ASTConstant constant;
    ASTSymbol symbol;
  };
};

typedef struct ASTOperand ASTOperand;

ASTOperand ASTOperandContant(ASTConstant value);
ASTOperand ASTOperandSymbol(ASTSymbol symbol);
bool ASTOperandEqual(ASTOperand op1, ASTOperand op2);

#define OPERAND_MAX_STRLEN 32

void ASTOperandToString(ASTOperand operand, char* buffer);

#endif /* astoperand_h */
