/* zt-driver.h */

#ifndef ZT_DRIVER_H
#define ZT_DRIVER_H

/* ----------------------------------------------------------------------- */

#include "zt-lex.h"
#include "zt-ast.h"

/* ----------------------------------------------------------------------- */

#define ZTMAXERRBUF 100

/* ----------------------------------------------------------------------- */

typedef struct ztparseinfo
{
  ztast_t *ast;
  char    *errbuf;
}
ztparseinfo_t;

/* ----------------------------------------------------------------------- */

ztast_t *ztast_from_file(const char *filename, char errbuf[ZTMAXERRBUF]);

/* ----------------------------------------------------------------------- */

#endif /* ZT_DRIVER_H */

/* vim: set ts=8 sts=2 sw=2 et: */
