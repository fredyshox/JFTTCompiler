#ifndef astprogram_h
#define astprogram_h

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "ast/node.h"

// Declaration

#define kASTIdentifierSimple 0
#define kASTIdentifierArray 1

struct ASTDeclaration {
  char* identifier;
  int type;
  int lower;
  int upper;
};

typedef struct ASTDeclaration ASTDeclaration;

ASTDeclaration ASTDeclarationCreateSimple(char* indentifier);
ASTDeclaration ASTDeclarationCreateArray(char* identifier, int lower, int upper);

// Declaration list
struct ASTDeclarationList {
  ASTDeclaration value;
  struct ASTDeclarationList* next;
};

typedef struct ASTDeclarationList ASTDeclarationList;

ASTDeclarationList* ASTDeclarationListCreate(ASTDeclaration decl);
void ASTDeclarationListFree(ASTDeclarationList* decl);

// Index

#define kASTIndexIdentifier 0
#define kASTIndexValue 1

struct ASTIndex {
  int indexType;
  union {
    char* identifier;
    int value;
  };
};

typedef struct ASTIndex ASTIndex;

ASTIndex ASTIndexCreateIdentifier(char* identifier);
ASTIndex ASTIndexCreateValue(int value);
bool ASTIndexEqual(ASTIndex i1, ASTIndex i2);

// Symbol

struct ASTSymbol {
  char* identifier;
  int type;
  ASTIndex index;
};

typedef struct ASTSymbol ASTSymbol;

ASTSymbol ASTSymbolCreateSimple(char* identifier);
ASTSymbol ASTSymbolCreateArray(char* identifier, ASTIndex index);
bool ASTSymbolEqual(ASTSymbol s1, ASTSymbol s2);
void ASTSymbolToString(ASTSymbol symbol, char* buffer);

// Program

struct ASTProgram {
  ASTDeclarationList* declarations;
  ASTNode* start;
};

typedef struct ASTProgram ASTProgram;

ASTProgram* ASTProgramCreate(ASTDeclarationList* dec, ASTNode* start);
void ASTProgramFree(ASTProgram* program);

#endif /* astprogram_h */
