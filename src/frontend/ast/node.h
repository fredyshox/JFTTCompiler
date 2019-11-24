#ifndef astnode_h
#define astnode_h

#include <stdlib.h>

enum ASTNodeType {
  kNodeAssignment,
  kNodeBranch,
  kNodeIO,
  kNodeWhileLoop,
  kNodeForLoop
};

typedef enum ASTNodeType ASTNodeType;

struct ASTNode {
  int type;
  void* command;
  struct ASTNode* next;
  struct ASTNode* prev;
};

typedef struct ASTNode ASTNode;

ASTNode* ASTNodeCreate(int type, void* command);
void ASTNodeFree(ASTNode* node);
void ASTNodeFreeList(ASTNode* node);

#endif /* ASTNode */
