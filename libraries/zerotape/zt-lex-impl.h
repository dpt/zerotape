/* zt-lex-impl.h */

#ifndef ZT_LEX_IMPL_H
#define ZT_LEX_IMPL_H

/* ----------------------------------------------------------------------- */

#ifndef NELEMS
#define NELEMS(A) (sizeof(A) / sizeof(A[0]))
#endif

/* ----------------------------------------------------------------------- */

struct ztlex
{
  FILE           *file;

  const char     *string;
  size_t          length;
  size_t          index; /* an index into 'string' */

  char            lexeme[MAXLEXEME];

  int             line;
  int             column;
  int             prevcolumn;

  int           (*getC)(struct ztlex *);
  void          (*ungetC)(int c, struct ztlex *);

  ztlex_freefn_t *freefn;

  ztlexinf_t      info; /* exposed to users as const * */
};

/* ----------------------------------------------------------------------- */

#endif /* ZT_LEX_IMPL_H */

/* vim: set ts=8 sts=2 sw=2 et: */
