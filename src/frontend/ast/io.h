#ifndef astio_h
#define astio_h

#include <stdlib.h>
#include "ast/operand.h"
#include "ast/types.h"

#define kIOReadCommand 0
#define kIOWriteCommand 1

struct ASTIO {
  int type;
  ASTOperand operand;
};

typedef struct ASTIO ASTIO;

ASTIO* ASTIOCreateRead(ASTSymbol symbol);
ASTIO* ASTIOCreateWrite(ASTOperand operand);
void ASTIOToString(ASTIO* io, char* buffer);

#endif /* astio_h */
