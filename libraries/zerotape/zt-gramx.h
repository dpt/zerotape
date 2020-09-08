/* zt-gramx.h */
/* header for Lemon generated parser */

#ifndef ZT_GRAMX_H
#define ZT_GRAMX_H

void *ztparseAlloc(void *(*mallocProc)(size_t), ztparser_t *pParse);
void ztparse(void *yyp, int yymajor, ztlexinf_t *yyminor);
void ztparseFree(void *p, void (*freeProc)(void *));

#endif /* ZT_GRAMX_H */

/* vim: ts=8 sts=2 sw=2 et */
