#include "operand.h"

ASTOperand ASTOperandContant(ASTConstant value) {
  ASTOperand operand;
  operand.type = kOperandConstant;
  operand.constant = value;

  return operand;
}

ASTOperand ASTOperandSymbol(ASTSymbol symbol) {
  ASTOperand operand;
  operand.type = kOperandSymbol;
  operand.symbol = symbol;

  return operand;
}

bool ASTOperandEqual(ASTOperand op1, ASTOperand op2) {
  if (op1.type == kOperandConstant && op2.type == kOperandConstant) {
    return op1.constant == op2.constant;
  } else if (op1.type == kOperandSymbol && op2.type == kOperandSymbol) {
    return ASTSymbolEqual(op1.symbol, op2.symbol);
  }

  return false;
}

void ASTOperandToString(ASTOperand operand, char* buffer) {
  if (operand.type == kOperandConstant) {
    sprintf(buffer, "%llu", operand.constant);
  } else {
    ASTSymbolToString(operand.symbol, buffer);
  }
}
