/* zt-gramx.h */
/* header for Lemon generated parser */

#ifndef ZT_GRAMX_H
#define ZT_GRAMX_H

#include <stdio.h>

void *ztparseAlloc(void *(*mallocProc)(size_t), ztparseinfo_t *info);
void ztparse(void *yyp, int yymajor, ztlexinf_t *yyminor);
void ztparseFree(void *p, void (*freeProc)(void *));
void ztparseTrace(FILE *TraceFILE, char *zTracePrompt);

#endif /* ZT_GRAMX_H */

/* vim: set ts=8 sts=2 sw=2 et: */
