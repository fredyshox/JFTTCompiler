//
//  frontend.c
//  jftt-frontend
//
//  Created by Kacper Raczy on 03.01.2019.
//

#include "frontend.h"

ASTProgram* parser_parseFile(const char* path) {
  FILE* fp = fopen(path, "r");
  if (fp == NULL) {
    fprintf(stderr, "Parser error: unable to open file at path %s\n", path);
    return NULL;
  }

  ASTProgram** ptr = malloc(sizeof(ASTProgram*));
  *ptr = NULL;
  if (yyparse(ptr, fp) != 0) {
    fprintf(stderr, "Parser error: cannot parse ASTree\n");
  }

  fclose(fp);
  return *ptr;
}

ASTProgram* parser_parseString(const char* str) {
  LEXER_BUFFER_STATE state = lexer_setScanString(str);
  ASTProgram** ptr = malloc(sizeof(ASTProgram*));
  *ptr = NULL;
  if (yyparse(ptr, NULL) != 0) {
      fprintf(stderr, "Parser error: cannot parse ASTree\n");
  }

  lexer_deleteScanBuffer(state);
  return *ptr;
}

int lexer_lex() {
  return yylex();
}

LEXER_BUFFER_STATE lexer_setScanString(const char* str) {
  return yy_scan_string(str);
}

void lexer_deleteScanBuffer(LEXER_BUFFER_STATE state) {
  yy_delete_buffer(state);
}
