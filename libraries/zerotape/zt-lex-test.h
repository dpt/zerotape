/* zt-lex-test.h */

#ifndef ZT_LEX_TEST_H
#define ZT_LEX_TEST_H

#include "zt-lex.h"

const char *ztlex_tokname(ztlextok_t t);
size_t ztlex_get_cursor(const ztlex_t *t);
int ztlex_stringtest(const char *string);
int ztlex_selftest(void);
int ztlex_dump_filename_to_tokens(const char *filename);

#endif /* ZT_LEX_TEST_H */

/* vim: set ts=8 sts=2 sw=2 et: */
