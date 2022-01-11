/* tests.c */

#include <stdio.h>

#include "fortify/fortify.h"

#include "../../libraries/zerotape/zt-ast.h"
#include "../../libraries/zerotape/zt-driver.h"
#include "../../libraries/zerotape/zt-lex-test.h"

static ztresult_t parse_and_dump_dot(const char *filename,
                                     const char *dotfilename)
{
  ztresult_t rc = ztresult_OK;
  
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

int main(int argc, char *argv[])
{
  ztresult_t rc = ztresult_OK;

  (void) Fortify_EnterScope();

  ztlex_selftest();

  ztlex_stringtest("");
  ztlex_stringtest(" ");
  ztlex_stringtest("x = 0;");
  ztlex_stringtest("y = [ 1 ];");
  ztlex_stringtest("()*+,-/;=[]{}"); /* no colon */
  ztlex_stringtest("1.23");
  ztlex_stringtest("$FF");
  ztlex_stringtest("0xFF");
  ztlex_stringtest("255");
  ztlex_stringtest("potato");
  ztlex_stringtest("nil");
  ztlex_stringtest("1nil2");
  ztlex_stringtest(" 1 nil 2 ");

  if (argc > 1)
  {
    rc = parse_and_dump_dot(argv[1], "ztast.dot");
    if (rc != ztresult_OK)
      fprintf(stderr, "parse_and_dump_dot() returned error %x\n", rc);
    else
      printf("ztast.dot created\n");
  }

  (void) Fortify_LeaveScope();

#ifdef FORTIFY
  printf("Fortify is enabled\n");
  Fortify_DumpAllMemory();
#endif

  return (rc == ztresult_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim: set ts=8 sts=2 sw=2 et: */
