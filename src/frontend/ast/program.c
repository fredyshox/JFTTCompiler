#include "program.h"

// Program

ASTProgram* ASTProgramCreate(ASTDeclarationList* dec, ASTNode* start) {
  ASTProgram* program = malloc(sizeof(ASTProgram));
  program->start = start;
  program->declarations = dec;

  return program;
}

void ASTProgramFree(ASTProgram* program) {
  ASTDeclarationListFree(program->declarations);
  ASTNode* current = program->start;
  ASTNode* next;
  while (current != NULL) {
    next = current->next;
    ASTNodeFree(current);
    current = next;
  }
  free(program);
}

// Declaration

ASTDeclaration ASTDeclarationCreateSimple(char* identifier) {
  ASTDeclaration declaration;
  declaration.identifier = identifier;
  declaration.type = kASTIdentifierSimple;

  return declaration;
}

ASTDeclaration ASTDeclarationCreateArray(char* identifier, int lower, int upper) {
  ASTDeclaration declaration;
  declaration.identifier = identifier;
  declaration.type = kASTIdentifierArray;
  declaration.lower = lower;
  declaration.upper = upper;

  return declaration;
}

void ASTDeclarationListFree(ASTDeclarationList* decl) {
  ASTDeclarationList* current = decl;
  ASTDeclarationList* next;
  while (current != NULL) {
    next = current->next;
    free(current);
    current = next;
  }
}

// Declaration list

ASTDeclarationList* ASTDeclarationListCreate(ASTDeclaration decl) {
  ASTDeclarationList* list = malloc(sizeof(ASTDeclarationList));
  list->value = decl;
  list->next = NULL;

  return list;
}

// Index

ASTIndex ASTIndexCreateIdentifier(char* identifier) {
  ASTIndex index;
  index.indexType = kASTIndexIdentifier;
  index.identifier = identifier;

  return index;
}

ASTIndex ASTIndexCreateValue(int value) {
  ASTIndex index;
  index.indexType = kASTIndexValue;
  index.value = value;

  return index;
}

bool ASTIndexEqual(ASTIndex i1, ASTIndex i2) {
  if (i1.indexType == kASTIndexValue && i2.indexType == kASTIndexValue) {
    return i1.value == i2.value;
  } else if (i1.indexType == kASTIndexIdentifier && i2.indexType == kASTIndexIdentifier) {
    return strcmp(i1.identifier, i2.identifier) == 0;
  }

  return false;
}

// Symbol

ASTSymbol ASTSymbolCreateSimple(char* identifier) {
  ASTSymbol symbol;
  symbol.identifier = identifier;
  symbol.type = kASTIdentifierSimple;

  return symbol;
}

ASTSymbol ASTSymbolCreateArray(char* identifier, ASTIndex index) {
  ASTSymbol symbol;
  symbol.identifier = identifier;
  symbol.type = kASTIdentifierArray;
  symbol.index = index;

  return symbol;
}

bool ASTSymbolEqual(ASTSymbol s1, ASTSymbol s2) {
  if (s1.type == kASTIdentifierSimple && s2.type == kASTIdentifierSimple) {
    return strcmp(s1.identifier, s2.identifier) == 0;
  } else if (s1.type == kASTIdentifierArray && s2.type == kASTIdentifierArray) {
    return strcmp(s1.identifier, s2.identifier) == 0 && ASTIndexEqual(s1.index, s2.index);
  }

  return false;
}

void ASTSymbolToString(ASTSymbol symbol, char* buffer) {
  if (symbol.type == kASTIdentifierSimple) {
    strcpy(buffer, symbol.identifier);
    return;
  }

  if (symbol.index.indexType == kASTIndexValue) {
    sprintf(buffer, "%s(%d)", symbol.identifier, symbol.index.value);
  } else if (symbol.index.indexType == kASTIndexIdentifier) {
    sprintf(buffer, "%s(%s)", symbol.identifier, symbol.index.identifier);
  }
}
