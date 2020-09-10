/* zt-parser.h */

#ifndef ZT_PARSER_H
#define ZT_PARSER_H

/* ----------------------------------------------------------------------- */

#include "zt-lex.h"
#include "zt-ast.h"

/* ----------------------------------------------------------------------- */

typedef struct ztparser
{
  ztlex_t *lexer;
  ztast_t *ast;
  int      syntax_error;
}
ztparser_t;

/* ----------------------------------------------------------------------- */

ztast_t *ztparser_from_file(const char *filename);

/* ----------------------------------------------------------------------- */

#endif /* ZT_PARSER_H */

/* vim: set ts=8 sts=2 sw=2 et: */
