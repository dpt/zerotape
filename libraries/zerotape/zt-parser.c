/* zt-parser.c */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "fortify.h"

#include "zt-lex.h"

#include "zt-ast.h"
#include "zt-gram.h"
#include "zt-parser.h"
#include "zt-gramx.h"

/* ----------------------------------------------------------------------- */

#define parser_malloc malloc
#define parser_free   free

/* ----------------------------------------------------------------------- */

static void ztparser_log(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

/* ----------------------------------------------------------------------- */

static void ztparser_init(ztparser_t *ps, ztlex_t *lex, ztast_t *ast)
{
  ps->lexer        = lex;
  ps->ast          = ast;
  ps->syntax_error = 0;
}

ztast_t *ztparser_from_file(const char *filename)
{
  ztlex_t          *lexer;
  ztparser_t        state;
  void             *parser;
  ztast_t          *ast;
  ztlextok_t        token;
  const ztlexinf_t *info;

  lexer = ztlex_from_file(parser_malloc, parser_free, filename);
  if (lexer == NULL)
    return NULL;

  parser = ztparseAlloc(parser_malloc, &state);
  ast = ztast_create(parser_malloc, parser_free, ztparser_log);
  ztparser_init(&state, lexer, ast);
  while (ztlex_next_token(lexer, &token, &info))
  {
    ztparse(parser, token, (ztlexinf_t *) info); /* cast away const for interface */
    if (state.syntax_error)
      goto stop;
  }
  ztparse(parser, 0, (ztlexinf_t *) info);

stop:
  ztparseFree(parser, parser_free);
  ztlex_destroy(lexer);
  return (state.syntax_error) ? NULL : ast;
}

/* ----------------------------------------------------------------------- */

/* vim: ts=8 sts=2 sw=2 et */
