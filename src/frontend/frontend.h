//
//  frontend.h
//  jftt-frontend
//
//  Created by Kacper Raczy on 03.01.2019.
//

#ifndef frontend_h
#define frontend_h

#include <stdio.h>
#include "ast/ast.h"
#include "parser.h"

// Parser
ASTProgram* parser_parseFile(const char* path);
ASTProgram* parser_parseString(const char* str);

// Lexer
typedef struct yy_buffer_state* LEXER_BUFFER_STATE;

int lexer_lex();
LEXER_BUFFER_STATE lexer_setScanString(const char* str);
void lexer_deleteScanBuffer(LEXER_BUFFER_STATE state);

#endif /* frontend_h */
