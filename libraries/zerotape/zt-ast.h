/* zt-ast.h */

#ifndef ZT_AST_H
#define ZT_AST_H

#include <stddef.h>

#include "zerotape/zerotape.h"

/* ----------------------------------------------------------------------- */

typedef void *(ztast_mallocfn_t)(size_t, void *opaque);
typedef void (ztast_freefn_t)(void *, void *opaque);

typedef void (ztast_logfn_t)(const char *fmt, ...);

/* ----------------------------------------------------------------------- */

typedef struct ztast ztast_t;

ztast_t *ztast_create(ztast_mallocfn_t  *mallocfn,
                      ztast_freefn_t    *freefn,
                      void              *opaque,
                      ztast_logfn_t     *logfn);
void ztast_destroy(ztast_t *ast);

/* ----------------------------------------------------------------------- */

struct ztast
{
  /* root node */
  ztast_program_t   *program;

  /* virtual functions */
  ztast_mallocfn_t  *mallocfn;
  ztast_freefn_t    *freefn;
  void              *opaque;
  
#ifdef ZTAST_LOG
  ztast_logfn_t     *logfn;
#endif
};

/* ----------------------------------------------------------------------- */

ztast_program_t *ztast_program(ztast_t *ast, ztast_statement_t *statement);

void ztast_statement_append(ztast_t           *ast,
                            ztast_statement_t *statementlist,
                            ztast_statement_t *appendee);
ztast_statement_t *ztast_statement_from_assignment(ztast_t            *ast,
                                                   ztast_assignment_t *ass);

ztast_assignment_t *ztast_assignment(ztast_t      *ast,
                                     ztast_id_t   *id,
                                     ztast_expr_t *expr);

ztast_id_t *ztast_id(ztast_t *ast, const char *name);

ztast_value_t *ztast_value_from_integer(ztast_t *ast, int integer);
ztast_value_t *ztast_value_from_decimal(ztast_t *ast, int decimal);
ztast_value_t *ztast_value_nil(ztast_t *ast);

ztast_expr_t *ztast_expr_from_value(ztast_t *ast, ztast_value_t *value);
ztast_expr_t *ztast_expr_from_scope(ztast_t *ast, ztast_scope_t *scope);
ztast_expr_t *ztast_expr_from_intarray(ztast_t *ast, ztast_intarray_t *array);
ztast_expr_t *ztast_expr_from_scopearray(ztast_t *ast, ztast_scopearray_t *scope);

ztast_scope_t *ztast_scope(ztast_t *ast, ztast_statement_t *statement);

ztast_intarray_t *ztast_intarray(ztast_t *ast, ztast_intarrayinner_t *elem);

/* call with (inner == NULL) to create */
ztast_intarrayinner_t *ztast_intarrayinner_append(ztast_t               *ast,
                                                  ztast_intarrayinner_t *inner,
                                                  int                    value);

ztast_scopearray_t *ztast_scopearray(ztast_t *ast, ztast_scopearrayinner_t *elem);

/* call with (inner == NULL) to create */
ztast_scopearrayinner_t *ztast_scopearrayinner_append(ztast_t                 *ast,
                                                      ztast_scopearrayinner_t *inner,
                                                      ztast_scope_t           *scope);

/* ----------------------------------------------------------------------- */

#ifdef ZT_DEBUG
/* Walks the AST, building a dot format graph. */
ztresult_t ztast_show(ztast_t *ast, const char *filename);
#endif

/* ----------------------------------------------------------------------- */

#endif /* ZT_AST_H */

/* vim: set ts=8 sts=2 sw=2 et: */
