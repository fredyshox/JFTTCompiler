%code requires {
  #include <stdio.h>
  #include <stdint.h>
  #include "ast/ast.h"
  #include "ast/expression.h"
  #include "ast/condition.h"

  struct ExprVal {
    int type;
    union {
      ASTExpression expression;
      ASTOperand operand;
    };
  };

  typedef struct ExprVal ExprVal;

  struct CommandList {
    ASTNode* head;
    ASTNode* tail;
  };

  typedef struct CommandList CommandList;

  struct DecList {
    ASTDeclarationList* head;
    ASTDeclarationList* tail;
  };

  typedef struct DecList DecList;

  struct yy_buffer_state;
  extern int yylex();
  extern struct yy_buffer_state* yy_scan_string(const char *str);
  extern void yy_delete_buffer(struct yy_buffer_state* buffer);
}

%code provides {
  ExprVal singleOperandExpression(ASTOperand op);
  ExprVal doubleOperandExpression(ASTExpression expr);
  CommandList newlist(ASTNode* head);
}

%code {
  void yyerror(ASTProgram** program, FILE* fp, const char* msg);
  extern FILE* yyin;
}

%error-verbose
%locations

/* pointer for ASTProgram */
%parse-param { ASTProgram** program }
%parse-param { FILE* finput }
%require "3.2.2"
%initial-action {
  if (finput != NULL) {
    yyin = finput;
  } else {
    yyin = stdin;
  }
}

/* terminal/nonterminal types */
%union {
  ASTNode* node;
  ASTOperand operand;
  ASTExpression expr;
  ASTCondition cond;
  ASTSymbol symbol;
  ASTDeclarationList* decl;
  DecList declist;
  CommandList cmdlist;
  ExprVal exprval;
  char* s;
  int i;
  int64_t ll;
}

/* token/type - type associations */
/* keywords */
%token DECLARE IN END
%token READ WRITE
%token IF THEN ELSE ENDIF
%token WHILE DO ENDWHILE ENDDO
%token FOR FROM TO DOWNTO ENDFOR
%token ERROR
/* operators */
%token SEMICOLON COLON ASSIGN
%token ADD SUB MUL DIV MOD
%token EQ NEQ LT GT LE GE
%token OPEN CLOSE
/* number and identifiers */
%token <s> ID
%token <ll> NUMBER
/* nonterminals */
%type <node> command
%type <cmdlist> commands
%type <operand> value
%type <exprval> expression
%type <expr> op_expression
%type <cond> condition
%type <decl> declaration
%type <declist> declarations
%type <symbol> identifier
%type <ll> number

%%

program: DECLARE declarations IN commands END {
    ASTDeclarationList* dlist = $2.head;
    ASTNode* current = $4.head;
    *program = ASTProgramCreate(dlist, current);
  }
;

  /* Type: DecList */
declarations: {
    DecList list;
    list.head = NULL;
    list.tail = NULL;
    $$ = list;
  }
  | declarations declaration {
    if ($1.head == NULL) {
      $1.head = $2;
      $1.tail = $2;
    } else {
      $1.tail->next = $2;
      $1.tail = $2;
    }

    $$ = $1;
  }
;

declaration: ID SEMICOLON {
    ASTDeclaration declaration = ASTDeclarationCreateSimple($1);
    $$ = ASTDeclarationListCreate(declaration);
  }
  | ID OPEN NUMBER COLON NUMBER CLOSE SEMICOLON {
    ASTDeclaration declaration = ASTDeclarationCreateArray($1, $3, $5);
    $$ = ASTDeclarationListCreate(declaration);
  }
;

  /* Type: ASTNode */
commands: commands command {
    CommandList list = $1;
    ASTNode* next = $2;
    list.tail->next = next;
    next->prev = list.tail;
    list.tail = next;
    $$ = list;
  }
  | command { $$ = newlist($1); }
;

  /* Type: ASTNode */
command: identifier ASSIGN expression SEMICOLON {
    ASTAssignment* a;
    if ($3.type == kRTypeExpression) {
      a = ASTAssignmentCreateWithExpression($1, $3.expression);
    } else {
      a = ASTAssignmentCreateWithOperand($1, $3.operand);
    }
    $$ = ASTNodeCreate(kNodeAssignment, a);
  }
  | IF condition THEN commands ENDIF {
    ASTBranch* branch = ASTBranchCreate($2, $4.head);
    $$ = ASTNodeCreate(kNodeBranch, branch);
  }
  | IF condition THEN commands ELSE commands ENDIF {
    ASTBranch* branch = ASTBranchCreateWithElse($2, $4.head, $6.head);
    $$ = ASTNodeCreate(kNodeBranch, branch);
  }
  | WHILE condition DO commands ENDWHILE {
    ASTWhileLoop* loop = ASTLoopCreateWhile($2, $4.head);
    $$ = ASTNodeCreate(kNodeWhileLoop, loop);
  }
  | DO commands WHILE condition ENDDO {
    ASTWhileLoop* loop = ASTLoopCreateDoWhile($4, $2.head);
    $$ = ASTNodeCreate(kNodeWhileLoop, loop);
  }
  | FOR ID FROM value TO value DO commands ENDFOR {
    ASTForLoop* loop = ASTLoopCreateForTo($2, $4, $6, $8.head);
    $$ = ASTNodeCreate(kNodeForLoop, loop);
  }
  | FOR ID FROM value DOWNTO value DO commands ENDFOR {
    ASTForLoop* loop = ASTLoopCreateForDownTo($2, $4, $6, $8.head);
    $$ = ASTNodeCreate(kNodeForLoop, loop);
  }
  | READ identifier SEMICOLON {
    ASTIO* io = ASTIOCreateRead($2);
    $$ = ASTNodeCreate(kNodeIO, io);
  }
  | WRITE value SEMICOLON {
    ASTIO* io = ASTIOCreateWrite($2);
    $$ = ASTNodeCreate(kNodeIO, io);
  }
;

  /* ExprVal */
  /* Single of double operand expression */
expression: value { $$ = singleOperandExpression($1); }
  | op_expression { $$ = doubleOperandExpression($1); }
;

condition: value EQ value { $$ = ASTConditionCreate($1, $3, kCondOperatorEqual); }
  | value NEQ value { $$ = ASTConditionCreate($1, $3, kCondOperatorNotEqual); }
  | value LT value { $$ = ASTConditionCreate($1, $3, kCondOperatorLess); }
  | value GT value { $$ = ASTConditionCreate($1, $3, kCondOperatorGreater); }
  | value GE value { $$ = ASTConditionCreate($1, $3, kCondOperatorGreaterEqual); }
  | value LE value { $$ = ASTConditionCreate($1, $3, kCondOperatorLessEqual); }
;

  /* Type: ASTExpression */
  /* Expression with operator */
op_expression: value ADD value { $$ = ASTExpressionCreate($1, $3, kOperatorAdd); }
  | value SUB value { $$ = ASTExpressionCreate($1, $3, kOperatorSub); }
  | value MUL value { $$ = ASTExpressionCreate($1, $3, kOperatorMul); }
  | value DIV value { $$ = ASTExpressionCreate($1, $3, kOperatorDiv); }
  | value MOD value { $$ = ASTExpressionCreate($1, $3, kOperatorMod); }
;

  /* Type: ASTOperand */
value: number { $$ = ASTOperandConstant($1); }
  | identifier { $$ = ASTOperandSymbol($1); }
;

number: SUB NUMBER { $$ = -1 * $2; }
  | NUMBER { $$ = $1; }
;

  /* Type: char* */
identifier: ID { $$ = ASTSymbolCreateSimple($1); }
  | ID OPEN ID CLOSE {
    ASTIndex index = ASTIndexCreateIdentifier($3);
    $$ = ASTSymbolCreateArray($1, index);
  }
  | ID OPEN NUMBER CLOSE {
    ASTIndex index = ASTIndexCreateValue($3);
    $$ = ASTSymbolCreateArray($1, index);
  }
;

%%

ExprVal singleOperandExpression(ASTOperand op) {
  ExprVal val;
  val.type = kRTypeOperand;
  val.operand = op;

  return val;
}

ExprVal doubleOperandExpression(ASTExpression expr) {
  ExprVal val;
  val.type = kRTypeExpression;
  val.expression = expr;

  return val;
}

CommandList newlist(ASTNode* head) {
  CommandList cmdlist;
  cmdlist.head = head;
  cmdlist.tail = head;
  return cmdlist;
}

void yyerror(ASTProgram** program, FILE* fp, const char* msg) {
  fprintf(stderr, "%d.%d-%d.%d: %s\n", yylloc.first_line, yylloc.first_column,
    yylloc.last_line, yylloc.last_column, msg);
}
