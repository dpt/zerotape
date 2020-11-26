/* zt-driver.c */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "fortify/fortify.h"

#include "zt-lex.h"

#include "zt-ast.h"
#include "zt-slab-alloc.h"
#include "zt-gram.h"
#include "zt-driver.h"
#include "zt-gramx.h"

/* ----------------------------------------------------------------------- */

static void *lexer_malloc(size_t n)
{
  printf("lexer allocated %lu\n", n);
  return malloc(n);
}

static void lexer_free(void *p)
{
  free(p);
}

static void *parser_malloc(size_t n)
{
  printf("parser allocated %lu\n", n);
  return malloc(n);
}

static void parser_free(void *p)
{
  free(p);
}

/* ----------------------------------------------------------------------- */

static void ztparser_log(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

/* ----------------------------------------------------------------------- */

ztast_t *ztast_from_file(const char *filename)
{
  ztlex_t          *lexer;
  ztparseinfo_t     parseinfo;
  void             *parser;
  ztast_t          *ast;
  ztlextok_t        token;
  const ztlexinf_t *info;
  ztslaballoc_t    *astslaballoc;

  lexer = ztlex_from_file(lexer_malloc, lexer_free, filename);
  if (lexer == NULL)
    return NULL;

  parser = ztparseAlloc(parser_malloc, &parseinfo);
  astslaballoc = ztslaballoc_create();
  if (astslaballoc == NULL)
  {
    ztparseFree(parser, parser_free);
    return NULL;
  }
  ast = ztast_create(ztslaballoc, ztslabfree, astslaballoc, ztparser_log);
  if (ast == NULL)
  {
    goto stop;
  }
  parseinfo.ast = ast;
  parseinfo.syntax_error = NULL;
  while (ztlex_next_token(lexer, &token, &info))
  {
    ztparse(parser, token, (ztlexinf_t *) info); /* cast away const for interface */
    if (parseinfo.syntax_error)
      goto stop;
  }
  ztparse(parser, 0, (ztlexinf_t *) info);

stop:
  ztparseFree(parser, parser_free);
  ztslaballoc_spew(astslaballoc);
  ztlex_destroy(lexer);
  return (parseinfo.syntax_error) ? NULL : ast;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
