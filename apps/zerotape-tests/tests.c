/* tests.c */

#include <stdio.h>

#include "fortify.h"

#include "../../libraries/zerotape/zt-ast.h"
#include "../../libraries/zerotape/zt-parser.h"
#include "../../libraries/zerotape/zt-lex-test.h"

static ztresult_t parse_and_dump_dot(const char *filename,
                                     const char *dotfilename)
{
  ztresult_t rc;
  ztast_t   *ast;

  ast = ztparser_from_file(filename);
  if (ast == NULL)
    return ztresult_NO_PROGRAM;

  rc = ztast_walk(ast, NULL);
  if (rc)
    return rc;

  rc = ztast_show(ast, dotfilename);
  if (rc)
    return rc;

  ztast_destroy(ast);

  return rc;
}

int main(int argc, const char * argv[])
{
  ztresult_t rc;

  Fortify_EnterScope();

  ztlex_selftest();

  ztlex_stringtest("x = 0;");
  ztlex_stringtest("y = [ 1 ];");
  ztlex_stringtest("()*+,-/:;=[]{}");
  ztlex_stringtest("1.23");
  ztlex_stringtest("$FF");
  ztlex_stringtest("0xFF");
  ztlex_stringtest("255");
  ztlex_stringtest("potato");

  rc = parse_and_dump_dot("test.zt", "ztast.dot");

  Fortify_LeaveScope();

#ifdef FORTIFY
  printf("Fortify is enabled\n");
  Fortify_DumpAllMemory();
#endif

  return 0;
}

/* vim: set ts=8 sts=2 sw=2 et: */
