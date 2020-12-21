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

#include "zt-lex-test.h"

/* ----------------------------------------------------------------------- */

static void *lexer_malloc(size_t n)
{
#ifdef ZT_DEBUG
  printf("lexer allocated %lu\n", n);
#endif
  return malloc(n);
}

static void lexer_free(void *p)
{
  free(p);
}

static void *parser_malloc(size_t n)
{
#ifdef ZT_DEBUG
  printf("parser allocated %lu\n", n);
#endif
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

ztast_t *ztast_from_file(const char *filename, char errbuf[ZTMAXERRBUF])
{
  ztlex_t          *lexer;
  ztparseinfo_t     parseinfo;
  void             *parser;
  ztslaballoc_t    *slaballoc;
  ztast_t          *ast;
  ztlextok_t        token;
  const ztlexinf_t *info;

  errbuf[0] = '\0';

  /* Build a lexer */
  lexer = ztlex_from_file(lexer_malloc, lexer_free, filename);
  if (lexer == NULL)
    return NULL;

  /* Allocate a parser */
  parser = ztparseAlloc(parser_malloc, &parseinfo);

  /* Create a memory allocator */
  slaballoc = ztslaballoc_create();
  if (slaballoc == NULL)
  {
    ztparseFree(parser, parser_free);
    return NULL;
  }

  /* Create an AST */
  ast = ztast_create(ztslaballoc, ztslabfree, slaballoc, ztparser_log);
  if (ast == NULL)
    goto stop;

  /* Setup parser */
  parseinfo.ast    = ast;
  parseinfo.errbuf = errbuf;

  /* Uncomment to enable parser debug output */
  /* ztparseTrace(stderr, "ztparse: "); */

  /* Feed the parser one token at a time */
  while (ztlex_next_token(lexer, &token, &info))
  {
    ztparse(parser, token, (ztlexinf_t *) info); /* cast away const for interface */
    if (parseinfo.errbuf[0])
      goto stop;
  }
  ztparse(parser, 0, (ztlexinf_t *) info);

stop:
  ztparseFree(parser, parser_free);
#ifdef ZT_DEBUG
  ztslaballoc_spew(slaballoc);
#endif
  ztlex_destroy(lexer);

  return (parseinfo.errbuf[0] == '\0') ? ast : NULL;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
