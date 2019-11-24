
#include "assignment.h"

ASTAssignment* ASTAssignmentCreateWithExpression(ASTSymbol symbol, ASTExpression expression) {
  ASTAssignment* assign = malloc(sizeof(ASTAssignment));
  assign->symbol = symbol;
  assign->rtype = kRTypeExpression;
  assign->expression = expression;

  return assign;
}

ASTAssignment* ASTAssignmentCreateWithOperand(ASTSymbol symbol, ASTOperand operand) {
  ASTAssignment* assign = malloc(sizeof(ASTAssignment));
  assign->symbol = symbol;
  assign->rtype = kRTypeOperand;
  assign->operand = operand;

  return assign;
}

void ASTAssignmentToString(ASTAssignment* assignment, char* buffer) {
  char lval[OPERAND_MAX_STRLEN];
  ASTSymbolToString(assignment->symbol, lval);
  if (assignment->rtype == kRTypeOperand) {
    char rval[OPERAND_MAX_STRLEN];
    ASTOperandToString(assignment->operand, rval);
    sprintf(buffer, "%s := %s", lval, rval);
  } else if (assignment->rtype == kRTypeExpression) {
    char rval[OPERAND_MAX_STRLEN * 2];
    ASTExpressionToString(assignment->expression, rval);
    sprintf(buffer, "%s := %s", lval, rval);
  }
}
