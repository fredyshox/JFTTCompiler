#include "node.h"

ASTNode* ASTNodeCreate(int type, void* command) {
  ASTNode* node = malloc(sizeof(ASTNode));
  node->type = type;
  node->command = command;
  node->next = NULL;
  node->prev = NULL;

  return node;
}

void ASTNodeFree(ASTNode* node) {
  if (node->next != NULL) {
    node->next->prev = NULL;
  }
  if (node->prev != NULL) {
    node->prev->next = NULL;
  }
  free(node->command);
  free(node);
}

void ASTNodeFreeList(ASTNode* node) {
  ASTNode* temp1 = node;
  ASTNode* temp2;
  while (temp1 != NULL) {
    temp2 = temp1;
    temp1 = temp1->next;
    ASTNodeFree(temp2);
  }
}
