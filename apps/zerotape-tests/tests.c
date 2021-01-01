/* tests.c */

#include <stdio.h>

#include "fortify/fortify.h"

#include "../../libraries/zerotape/zt-ast.h"
#include "../../libraries/zerotape/zt-driver.h"
#include "../../libraries/zerotape/zt-lex-test.h"

static ztresult_t parse_and_dump_dot(const char *filename,
                                     const char *dotfilename)
{
  ztresult_t rc;
  char       errbuf[ZTMAXERRBUF];
  ztast_t   *ast;

  ast = ztast_from_file(filename, errbuf);
  if (ast == NULL)
  {
    fprintf(stderr, "parse error: %s\n", errbuf);
    return ztresult_NO_PROGRAM;
  }

#ifdef ZT_DEBUG
  rc = ztast_show(ast, dotfilename);
  if (rc)
    return rc;
#endif /* ZT_DEBUG */
  
  ztast_destroy(ast);

  return rc;
}

int main(int argc, const char *argv[])
{
  ztresult_t rc;

  Fortify_EnterScope();

  ztlex_selftest();

  ztlex_stringtest("");
  ztlex_stringtest(" ");
  ztlex_stringtest("x = 0;");
  ztlex_stringtest("y = [ 1 ];");
  ztlex_stringtest("()*+,-/:;=[]{}");
  ztlex_stringtest("1.23");
  ztlex_stringtest("$FF");
  ztlex_stringtest("0xFF");
  ztlex_stringtest("255");
  ztlex_stringtest("potato");
  ztlex_stringtest("nil");
  ztlex_stringtest("1nil2");
  ztlex_stringtest(" 1 nil 2 ");

  rc = parse_and_dump_dot("test.zt", "ztast.dot");
  if (rc)
  {
    fprintf(stderr, "parse_and_dump_dot() returned error %d\n", rc);
  }

  Fortify_LeaveScope();

#ifdef FORTIFY
  printf("Fortify is enabled\n");
  Fortify_DumpAllMemory();
#endif

  return 0;
}

/* vim: set ts=8 sts=2 sw=2 et: */
