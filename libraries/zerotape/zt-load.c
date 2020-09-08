/* zt-load.c */

#include <assert.h>
#include <stdio.h>

#include "zerotape/zerotape.h"

#include "zt-ast.h"
#include "zt-parser.h"
#include "zt-run.h"

ztresult_t zt_load(const ztstruct_t *meta,
                   void             *structure,
                   const char       *filename,
                   const ztregion_t *regions,
                   int               nregions)
{
  ztresult_t rc;
  ztast_t   *ast;
  char      *syntax_error;

  assert(meta);
  assert(structure);
  assert(filename);
  /* regions may be NULL */
  assert(nregions >= 0);

  ast = ztparser_from_file(filename);
  if (ast == NULL)
    return ztresult_PARSE_FAIL;

  rc = zt_run_program(ast,
                      meta,
                      regions,
                      nregions,
                      structure,
                      &syntax_error);
  if (rc != ztresult_OK)
    puts(syntax_error);

  return rc;
}

/* vim: ts=8 sts=2 sw=2 et */
