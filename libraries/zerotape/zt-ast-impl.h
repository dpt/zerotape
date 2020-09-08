/* zt-ast-impl.h */

#ifndef ZT_AST_IMPL_H
#define ZT_AST_IMPL_H

#include "zt-ast.h"

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

/* a program is a list of statements */
struct ztast_program
{
  struct ztast_statement *statements;
};

struct ztast_statement
{
  struct ztast_statement *next; /* linked list */
  enum ztast_statement_type
  {
    STMT_ASSIGNMENT
  }
  type;
  union ztast_statement_data
  {
    struct ztast_assignment *assignment;
  }
  u;
};

struct ztast_assignment
{
  struct ztast_id   *id;
  struct ztast_expr *expr;
};

struct ztast_id
{
  char name[1];
};

struct ztast_value
{
  enum ztast_value_type
  {
    VAL_INTEGER,
    VAL_DECIMAL
  }
  type;
  union ztast_value_data
  {
    int integer;
    int decimal;
  }
  data;
};

struct ztast_expr
{
  enum ztast_expr_type
  {
    EXPR_VALUE,
    EXPR_ARRAY,
    EXPR_SCOPE
  }
  type;
  union ztast_expr_data
  {
    struct ztast_value *value;
    struct ztast_array *array;
    struct ztast_scope *scope;
  }
  data;
};

struct ztast_array
{
  struct ztast_arrayelem *elems;
};

struct ztast_arrayelem
{
  struct ztast_arrayelem *next; /* linked list */
  ztast_arrayindex_t      index;
  struct ztast_expr      *expr;
};

struct ztast_scope
{
  struct ztast_statement *statements;
};

/* ----------------------------------------------------------------------- */

#endif /* ZT_AST_IMPL_H */

/* vim: ts=8 sts=2 sw=2 et */
