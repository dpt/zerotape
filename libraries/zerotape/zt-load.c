/* zt-load.c */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zerotape/zerotape.h"

#include "zt-ast.h"
#include "zt-driver.h"
#include "zt-run.h"

/* ----------------------------------------------------------------------- */

ztresult_t zt_load(const ztstruct_t  *meta,
                   void              *structure,
                   const char        *filename,
                   const ztregion_t  *regions,
                   int                nregions,
                   ztloader_t       **loaders,
                   int                nloaders,
                   char             **syntax_error)
{
  ztresult_t rc;
  ztast_t   *ast;
  char       errbuf[ZTMAXERRBUF] = "";

  assert(meta);
  assert(structure);
  assert(filename);
  /* regions may be NULL */
  assert(nregions >= 0);
  assert(syntax_error);

  *syntax_error = NULL;

  ast = ztast_from_file(filename, errbuf);
  if (ast == NULL)
  {
    rc = ztresult_PARSE_FAIL;
    goto exit;
  }

  rc = zt_run_program(ast,
                      meta,
                      regions,
                      nregions,
                      loaders,
                      nloaders,
                      structure,
                      errbuf);

  ztast_destroy(ast);

exit:
  if (rc && errbuf[0])
  {
    size_t len;

    len = strlen(errbuf) + 1;
    *syntax_error = malloc(len);
    if (*syntax_error)
      memcpy(*syntax_error, errbuf, len);
  }

  return rc;
}

/* ----------------------------------------------------------------------- */

void zt_freesyntax(char *syntax_error)
{
  free(syntax_error);
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
