// zt-gram.y
//
// Grammar for zerotape
//
//

// TODO: Store the lexeme location in the AST?
//{ printf("p: [%d,%d] %s\n", I->line, I->column, I->lexeme); }

%token_prefix ZTTOKEN_

%token_type { ztlexinf_t * }

%default_type { ztlextok_t }

%extra_context { ztparseinfo_t *info }

%syntax_error {
  if (TOKEN)
    fprintf(stderr, "error: syntax error at line %d column %d\n", TOKEN->line, TOKEN->column);
  else
    fprintf(stderr, "error: syntax error\n");
  info->syntax_error = 1;
}

%stack_overflow {
  fprintf(stderr, "error: parser stack overflow\n");
}

%name ztparse

%include {
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "zt-ast.h"
#include "zt-driver.h"
#include "zt-lex.h"
}

%left PLUS MINUS.
%left TIMES DIVIDE.

%type program { ztast_program_t * }
program(A)      ::= .                 { A = ztast_program(info->ast, NULL); }
program(A)      ::= statementlist(B). { A = ztast_program(info->ast, B); }

%type statementlist { ztast_statement_t * }
statementlist(A) ::= statementlist(A) statement(B). { ztast_statement_append(info->ast, A, B); }
statementlist   ::= statement.

%type statement { ztast_statement_t * }
statement(A)    ::= assignment(B). { A = ztast_statement_from_assignment(info->ast, B); }

%type assignment { ztast_assignment_t * }
assignment(A)   ::= id(B) EQUALS expr(C) SEMICOLON. { A = ztast_assignment(info->ast, B, C); }

%type id { ztast_id_t * }
id(A)           ::= NAME(B). { A = ztast_id(info->ast, B->lexeme); }

%type value { ztast_value_t * }
value(A)        ::= term(B).    { A = ztast_value_from_integer(info->ast, B); }
value(A)        ::= decimal(B). { A = ztast_value_from_decimal(info->ast, B); }
value(A)        ::= NIL.        { A = ztast_value_nil(info->ast); }

%type term { int }
term(A)         ::= term(B) PLUS term(C).   { A = B + C; }
term(A)         ::= term(B) MINUS term(C).  { A = B - C; }
term(A)         ::= term(B) TIMES term(C).  { A = B * C; }
term(A)         ::= term(B) DIVIDE term(C). { A = B / C; }
term(A)         ::= LPAREN term(E) RPAREN.  { A = E; }
term            ::= integer.

%type integer { int }
integer(A)      ::= INT(B).       { A = atoi(B->lexeme); }
integer(A)      ::= DOLLARHEX(B). { A = (int) strtol(B->lexeme + 1, NULL, 16); } // skip $
integer(A)      ::= HEX(B).       { A = (int) strtol(B->lexeme + 2, NULL, 16); } // skip 0x

%type decimal { int }
decimal(A)      ::= DECIMAL(B).   { A = (int)(atof(B->lexeme) * 100); }

%type expr { ztast_expr_t * }
expr(A)         ::= value(B). { A = ztast_expr_from_value(info->ast, B); }
expr(A)         ::= array(B). { A = ztast_expr_from_array(info->ast, B); }
expr(A)         ::= scope(B). { A = ztast_expr_from_scope(info->ast, B); }

%type array { ztast_array_t * }
array(A)        ::= LSQBRA RSQBRA.                  { A = ztast_array(info->ast, NULL); }
array(A)        ::= LSQBRA arrayelemlist(B) RSQBRA. { A = ztast_array(info->ast, B); }

// Note: This permits mixed formats for arrays which may not be expected. e.g. What does "jim = [ 10:$FF, 2 ];" mean?
%type arrayelemlist { ztast_arrayelem_t * }
arrayelemlist(A) ::= arrayelemlist(A) COMMA arrayelem(B). { ztast_arrayelem_append(info->ast, A, B); }
arrayelemlist   ::= arrayelem.

%type arrayelem { ztast_arrayelem_t * }
arrayelem(A)    ::= expr(B).                  { A = ztast_arrayelem(info->ast, -1, B); }
arrayelem(A)    ::= integer(B) COLON expr(C). { A = ztast_arrayelem(info->ast, B, C); }

%type scope { ztast_scope_t * }
scope(A)        ::= LBRACE RBRACE.                  { A = ztast_scope(info->ast, NULL); }
scope(A)        ::= LBRACE statementlist(B) RBRACE. { A = ztast_scope(info->ast, B); }
