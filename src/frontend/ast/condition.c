
#include "condition.h"

ASTCondition ASTConditionCreate(ASTOperand op1, ASTOperand op2, int operator) {
  ASTCondition condition;
  condition.operand1 = op1;
  condition.operand2 = op2;
  condition.operator = operator;

  return condition;
}

void ASTConditionToString(ASTCondition condition, char* buffer) {
  char opstr[3] = "\0\0\0";
  switch (condition.operator) {
    case kCondOperatorEqual:
      opstr[0] = '=';
      break;
    case kCondOperatorNotEqual:
      opstr[0] = '!'; opstr[1] = '=';
      break;
    case kCondOperatorGreater:
      opstr[0] = '>';
      break;
    case kCondOperatorLess:
      opstr[0] = '<';
      break;
    case kCondOperatorGreaterEqual:
      opstr[0] = '>'; opstr[1] = '=';
      break;
    default:
      opstr[0] = '<'; opstr[1] = '=';
      break;
  }

  char ostr1[OPERAND_MAX_STRLEN], ostr2[OPERAND_MAX_STRLEN];
  ASTOperandToString(condition.operand1, ostr1);
  ASTOperandToString(condition.operand2, ostr2);
  sprintf(buffer, "%s %s %s", ostr1, opstr, ostr2);
}
