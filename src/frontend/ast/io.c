
#include "io.h"

ASTIO* ASTIOCreateRead(ASTSymbol symbol) {
  ASTIO* io = malloc(sizeof(ASTIO));
  io->type = kIOReadCommand;
  io->operand = ASTOperandSymbol(symbol);

  return io;
}

ASTIO* ASTIOCreateWrite(ASTOperand operand) {
  ASTIO* io = malloc(sizeof(ASTIO));
  io->type = kIOWriteCommand;
  io->operand = operand;

  return io;
}

void ASTIOToString(ASTIO* io, char* buffer) {
  char op[OPERAND_MAX_STRLEN];
  ASTOperandToString(io->operand, op);
  if (io->type == kIOReadCommand) {
    sprintf(buffer, "READ %s", op);
  } else if (io->type == kIOWriteCommand) {
    sprintf(buffer, "WRITE %s", op);
  }
}
