%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

char whitespace[6] = " \t\n\r\v\f";
int yywrap();
int tokenFromKeyword(char* str);

%}
%option yylineno
  /* start conditions */
  /* inclusive (%s) - rules with no start condition + rules with this condition are matched */
  /* exclusive (%x) - rule with no start state is not matched when active */
%x comment

%%
  /* keywords */

READ {
  return READ;
}

WRITE {
  return WRITE;
}

DECLARE {
  return DECLARE;
}

IN {
  return IN;
}

IF {
  return IF;
}

THEN {
  return THEN;
}

ELSE {
  return ELSE;
}

WHILE {
  return WHILE;
}

DO {
  return DO;
}

FOR {
  return FOR;
}

FROM {
  return FROM;
}

TO {
  return TO;
}

DOWNTO {
  return DOWNTO;
}

ENDIF {
  return ENDIF;
}

ENDWHILE {
  return ENDWHILE;
}

ENDDO {
  return ENDDO;
}

ENDFOR {
  return ENDFOR;
}

END {
  return END;
}
  
  /* identifiers */
[_a-z]+ {
  // create cpy of yytext
  char* id_cpy = malloc(strlen(yytext) * sizeof(char));
  strcpy(id_cpy, yytext);
  yylval.s = id_cpy;
  return ID;
 }

  /* number */
[0-9]+ {
  // yylval is int64_t
  yylval.ll = strtoll(yytext, NULL, 10);
  return NUMBER;
}

  /* arithmetic operators */
\*  { return MUL; }
\+  { return ADD; }
-   { return SUB; }
\/  { return DIV; }
%   { return MOD; }

  /* assignment */
:= { return ASSIGN; }

  /* parentheses */
\( { return OPEN; }
\) { return CLOSE; }

  /* logical operators */
!=  { return NEQ; }
\<= { return LE; }
\>= { return GE; }
=   { return EQ; }
\<  { return LT; }
\>  { return GT; }
  /* other */
\n { }
[[:blank:]] { }
;  { return SEMICOLON; }
:  { return COLON; }
  /* comments */
\[ { BEGIN(comment); }
<comment>\] {  BEGIN(INITIAL); }
<comment>\[ { return ERROR; }
<comment>[^\]] { }
  /* anything else is invalid */
. { return ERROR; }
%%

int yywrap() {
    return 1;
}

