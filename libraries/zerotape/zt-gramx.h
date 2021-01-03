/* zt-gramx.h */
/* header for Lemon generated parser */

#ifndef ZT_GRAMX_H
#define ZT_GRAMX_H

#include <stdio.h>

#include "zt-driver.h"
#include "zt-lex.h"

void ztparseTrace(FILE *TraceFILE, char *zTracePrompt);
void ztparseInit(void *yypRawParser, ztparseinfo_t *info);
void *ztparseAlloc(void *(*mallocProc)(size_t), ztparseinfo_t *info);
void ztparseFinalize(void *p);
void ztparseFree(void *p, void (*freeProc)(void *));
void ztparse(void *yyp, int yymajor, ztlexinf_t *yyminor);
int ztparseFallback(int iToken);

#endif /* ZT_GRAMX_H */

/* vim: set ts=8 sts=2 sw=2 et: */
