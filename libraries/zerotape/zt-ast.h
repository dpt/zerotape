/* zt-ast.h */

#ifndef ZT_AST_H
#define ZT_AST_H

#include <stddef.h>

#include "zerotape/zerotape.h"

/* ----------------------------------------------------------------------- */

typedef void *(ztast_mallocfn_t)(size_t);
typedef void (ztast_freefn_t)(void *);
typedef void (ztast_logfn_t)(const char *fmt, ...);

/* ----------------------------------------------------------------------- */

typedef struct ztast ztast_t;

ztast_t *ztast_create(ztast_mallocfn_t *mallocfn,
                      ztast_freefn_t   *freefn,
                      ztast_logfn_t    *logfn);
void ztast_destroy(ztast_t *ast);

/* ----------------------------------------------------------------------- */

struct ztast
{
  /* root node */
  ztast_program_t  *program;

  /* virtual functions */
  ztast_mallocfn_t *mallocfn;
  ztast_freefn_t   *freefn;
#ifdef ZTAST_LOG
  ztast_logfn_t    *logfn;
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

ztast_expr_t *ztast_expr_from_value(ztast_t *ast, ztast_value_t *value);
ztast_expr_t *ztast_expr_from_array(ztast_t *ast, ztast_array_t *array);
ztast_expr_t *ztast_expr_from_scope(ztast_t *ast, ztast_scope_t *scope);

ztast_array_t *ztast_array(ztast_t *ast, ztast_arrayelem_t *elem);

ztast_arrayelem_t *ztast_arrayelem(ztast_t           *ast,
                                   ztast_arrayindex_t index,
                                   ztast_expr_t      *expr);
void ztast_arrayelem_append(ztast_t           *ast,
                            ztast_arrayelem_t *array,
                            ztast_arrayelem_t *elem);

ztast_scope_t *ztast_scope(ztast_t *ast, ztast_statement_t *statement);
void ztast_scope_append(ztast_t           *ast,
                        ztast_scope_t     *scope,
                        ztast_statement_t *stmt);

/* ----------------------------------------------------------------------- */

ztresult_t ztast_walk(ztast_t *ast, void *opaque);

/* ----------------------------------------------------------------------- */

ztresult_t ztast_show(ztast_t *ast, const char *filename);

/* ----------------------------------------------------------------------- */

#endif /* ZT_AST_H */

/* vim: set ts=8 sts=2 sw=2 et: */
