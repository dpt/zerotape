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
    sprintf(info->errbuf, "syntax error at line %d column %d", TOKEN->line, TOKEN->column);
  else
    sprintf(info->errbuf, "syntax error");
}

%stack_overflow {
  sprintf(info->errbuf, "parser stack overflow");
}

%name ztparse

%include {
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "zerotape/zerotape.h"

#include "zt-driver.h"
#include "zt-gramx.h"
#include "zt-lex.h"
}

%left PLUS MINUS.
%left TIMES DIVIDE.

%type program { ztast_program_t * }
program(A)      ::= .                 { A = ztast_program(info->ast, NULL); }
program(A)      ::= statementlist(B). { A = ztast_program(info->ast, B); }

%type statementlist { ztast_statement_t * }
statementlist(A)  ::= statementlist(A) statement(B). { ztast_statement_append(info->ast, A, B); }
statementlist     ::= statement.

%type statement { ztast_statement_t * }
statement(A)    ::= assignment(B). { A = ztast_statement_from_assignment(info->ast, B); }

%type assignment { ztast_assignment_t * }
assignment(A)   ::= id(B) EQUALS expr(C) SEMICOLON. { A = ztast_assignment(info->ast, B, C); }

%type id { ztast_id_t * }
id(A)           ::= NAME(B). { A = ztast_id(info->ast, B->lexeme); }

// While we allow expressions to be values, we don't allow them to be arrays of
// values, just arrays of int, scope, or a single scope.
%type expr { ztast_expr_t * }
expr(A)         ::= value(B).      { A = ztast_expr_from_value(info->ast, B); }
expr(A)         ::= scope(B).      { A = ztast_expr_from_scope(info->ast, B); }
expr(A)         ::= intarray(B).   { A = ztast_expr_from_intarray(info->ast, B); }
expr(A)         ::= scopearray(B). { A = ztast_expr_from_scopearray(info->ast, B); }

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

%type scope { ztast_scope_t * }
scope(A)        ::= LBRACE RBRACE.                  { A = ztast_scope(info->ast, NULL); }
scope(A)        ::= LBRACE statementlist(B) RBRACE. { A = ztast_scope(info->ast, B); }

%type intarray { ztast_intarray_t * }
intarray(A)     ::= LSQBRA RSQBRA.                  { A = ztast_intarray(info->ast, NULL); }
intarray(A)     ::= LSQBRA intarrayinner(B) RSQBRA. { A = ztast_intarray(info->ast, B); }

%type intarrayinner { ztast_intarrayinner_t * }
intarrayinner(A) ::= term(B). { A = ztast_intarrayinner_append(info->ast, NULL, B); }
intarrayinner(A) ::= intarrayinner(A) COMMA term(B). { A = ztast_intarrayinner_append(info->ast, A, B); }

%type scopearray { ztast_scopearray_t * }
scopearray(A)   ::= LSQBRA scopearrayinner(B) RSQBRA. { A = ztast_scopearray(info->ast, B); }

%type scopearrayinner { ztast_scopearrayinner_t * }
scopearrayinner(A) ::= scope(B). { A = ztast_scopearrayinner_append(info->ast, NULL, B); }
scopearrayinner(A) ::= scopearrayinner(A) COMMA scope(B). { A = ztast_scopearrayinner_append(info->ast, A, B); }
