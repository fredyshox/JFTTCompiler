#include "expression.h"

ASTExpression ASTExpressionCreate(ASTOperand op1, ASTOperand op2, int operator) {
  ASTExpression expr;
  expr.operand1 = op1;
  expr.operand2 = op2;
  expr.operator = operator;

  return expr;
}

void ASTExpressionToString(ASTExpression expr, char* buffer) {
    char c;
    switch (expr.operator) {
      case kOperatorAdd:
        c = '+';
        break;
      case kOperatorSub:
        c = '-';
        break;
      case kOperatorMul:
        c = '*';
        break;
      case kOperatorDiv:
        c = '/';
        break;
      default:
        c = '%';
        break;
    }
    char op1[OPERAND_MAX_STRLEN], op2[OPERAND_MAX_STRLEN];
    ASTOperandToString(expr.operand1, op1);
    ASTOperandToString(expr.operand2, op2);
    sprintf(buffer, "%s %c %s", op1, c, op2);
}
