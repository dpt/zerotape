/* zt-lex.h */

#ifndef ZT_LEX_H
#define ZT_LEX_H

#include <stddef.h>

/* ----------------------------------------------------------------------- */

#define MAXLEXEME 256

/* ----------------------------------------------------------------------- */

typedef struct ztlex ztlex_t;

typedef int ztlextok_t;

typedef struct ztlexinf
{
  int  line, column;
  int  length;
  char lexeme[MAXLEXEME];
}
ztlexinf_t;

/* ----------------------------------------------------------------------- */

typedef void *(ztlex_mallocfn_t)(size_t);
typedef void (ztlex_freefn_t)(void *);

/* ----------------------------------------------------------------------- */

ztlex_t *ztlex_from_file(ztlex_mallocfn_t *mallocfn,
                         ztlex_freefn_t   *freefn,
                   const char             *filename);
ztlex_t *ztlex_from_string(ztlex_mallocfn_t *mallocfn,
                           ztlex_freefn_t   *freefn,
                     const char             *string);
void ztlex_destroy(ztlex_t *lex);

/* Returns non-zero if a valid token */
int ztlex_next_token(ztlex_t     *lexer,
                     ztlextok_t  *token,
               const ztlexinf_t **info);

/* ----------------------------------------------------------------------- */

typedef int (ixlexfn_t)(ztlex_t *lex);

ixlexfn_t ztlex_isdollarhex;
ixlexfn_t ztlex_ishex;
ixlexfn_t ztlex_isdecimal;
ixlexfn_t ztlex_isinteger;
ixlexfn_t ztlex_isname;

/* ----------------------------------------------------------------------- */

#endif /* ZT_LEX_H */

/* vim: set ts=8 sts=2 sw=2 et: */
