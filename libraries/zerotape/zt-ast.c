/* zt-ast.c */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "fortify/fortify.h"

#include "zt-ast.h"

/* TODO
 *
 * might want to store the lexerinfo structures in the AST at some point
 */

/* ----------------------------------------------------------------------- */

#define ZTAST_MALLOC(SZ) ast->mallocfn(SZ, ast->opaque)
#define ZTAST_FREE(P)    do { if (ast->freefn) ast->freefn(P, ast->opaque); } while (0)

/* ----------------------------------------------------------------------- */

static void ztast_destroy_expr(ztast_t      *ast,
                               ztast_expr_t *expr);

static void ztast_destroy_value(ztast_t       *ast,
                                ztast_value_t *value);

/* ----------------------------------------------------------------------- */

ztast_t *ztast_create(ztast_mallocfn_t  *mallocfn,
                      ztast_freefn_t    *freefn,
                      void              *opaque,
                      ztast_logfn_t     *logfn)
{
  ztast_t *ast;

  ast = mallocfn(sizeof(*ast), opaque);
  if (ast == NULL)
    return NULL;

  ast->program  = NULL;
  
  ast->mallocfn = mallocfn;
  ast->freefn   = freefn;
  ast->opaque   = opaque;

#ifdef ZTAST_LOG
  ast->logfn     = logfn;
#endif

  return ast;
}

static void ztast_destroy_statements(ztast_t           *ast,
                                     ztast_statement_t *statements)
{
  ztast_statement_t *st;
  ztast_statement_t *next;

  if (statements == NULL)
    return; /* no statements */

  for (st = statements; st != NULL; st = next)
  {
    switch (st->type)
    {
    case ZTSTMT_ASSIGNMENT:
      ztast_destroy_expr(ast, st->u.assignment->expr);
      ZTAST_FREE(st->u.assignment->id);
      ZTAST_FREE(st->u.assignment);
      break;

    default:
      assert(0);
      break;
    }
    next = st->next;
    ZTAST_FREE(st);
  }
}

static void ztast_destroy_expr(ztast_t      *ast,
                               ztast_expr_t *expr)
{
  switch (expr->type)
  {
  case ZTEXPR_VALUE:
    ztast_destroy_value(ast, expr->data.value);
    break;

  case ZTEXPR_SCOPE:
    ztast_destroy_statements(ast, expr->data.scope->statements);
    ZTAST_FREE(expr->data.scope);
    break;

  case ZTEXPR_INTARRAY:
    {
      ztast_intarrayinner_t *inner;

      inner = expr->data.intarray->inner;
      if (inner)
      {
        ZTAST_FREE(inner->ints);
        ZTAST_FREE(inner);
      }
      ZTAST_FREE(expr->data.intarray);
    }
    break;

  case ZTEXPR_SCOPEARRAY:
    {
      ztast_scopearrayinner_t *inner;

      inner = expr->data.scopearray->inner;
      if (inner)
      {
        int i;

        for (i = 0; i < inner->nused; i++)
        {
          ztast_destroy_statements(ast, inner->scopes[i]->statements);
          ZTAST_FREE(inner->scopes[i]);
        }
        ZTAST_FREE(inner->scopes);
        ZTAST_FREE(inner);
      }
      ZTAST_FREE(expr->data.scopearray);
    }
    break;
  }

  ZTAST_FREE(expr);
}

static void ztast_destroy_value(ztast_t       *ast,
                                ztast_value_t *value)
{
  ZTAST_FREE(value);
}

void ztast_destroy(ztast_t *ast)
{
  if (ast == NULL)
    return;

  ztast_destroy_statements(ast, ast->program->statements);

  ZTAST_FREE(ast->program);
  ZTAST_FREE(ast);
}

/* ----------------------------------------------------------------------- */

ztast_program_t *ztast_program(ztast_t *ast, ztast_statement_t *statements)
{
  ztast_program_t *program;

  assert(ast);
  /* statement may be NULL */

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_program\n");
#endif

  program = ZTAST_MALLOC(sizeof(*program));
  if (program == NULL)
    return NULL;

  program->statements = statements;

  /* Store the initial program pointer in ast_t.
   * This is here because it's awkward to do in the parser. */
  assert(ast->program == NULL);
  ast->program = program;

  return program;
}

void ztast_statement_append(ztast_t           *ast,
                            ztast_statement_t *statementlist,
                            ztast_statement_t *appendee)
{
  ztast_statement_t *stmt;

  assert(ast);
  assert(statementlist);
  assert(appendee);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_statement_append %p to %p", appendee, statementlist);
#endif

  for (stmt = statementlist; stmt->next != NULL; stmt = stmt->next)
    ;
  stmt->next = appendee;

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn(" -> %p\n", stmt);
#endif
}

ztast_statement_t *ztast_statement_from_assignment(ztast_t            *ast,
                                                   ztast_assignment_t *assignment)
{
  ztast_statement_t *stmt;

  assert(ast);
  assert(assignment);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_statement_from_assignment %p", assignment);
#endif

  stmt = ZTAST_MALLOC(sizeof(*stmt));
  if (stmt == NULL)
    return NULL;

  stmt->next         = NULL;
  stmt->type         = ZTSTMT_ASSIGNMENT;
  stmt->u.assignment = assignment;

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn(" -> %p\n", stmt);
#endif

  return stmt;
}

ztast_assignment_t *ztast_assignment(ztast_t      *ast,
                                     ztast_id_t   *id,
                                     ztast_expr_t *expr)
{
  ztast_assignment_t *ass;

  assert(ast);
  assert(id);
  assert(expr);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_assignment\n");
#endif

  ass = ZTAST_MALLOC(sizeof(*ass));
  if (ass == NULL)
    return NULL;

  ass->id   = id;
  ass->expr = expr;

  return ass;
}

ztast_id_t *ztast_id(ztast_t *ast, const char *name)
{
  size_t      namelen;
  ztast_id_t *id;

  assert(ast);
  assert(name);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_id\n");
#endif

  namelen = strlen(name);
  id = ZTAST_MALLOC(offsetof(ztast_id_t, name) + namelen + 1);
  if (id == NULL)
    return NULL;

  memcpy(id->name, name, namelen + 1);

  return id;
}

ztast_expr_t *ztast_expr_from_value(ztast_t *ast, ztast_value_t *value)
{
  ztast_expr_t *expr;

  assert(ast);
  assert(value);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_expr_from_value\n");
#endif

  expr = ZTAST_MALLOC(sizeof(*expr));
  if (expr == NULL)
    return NULL;

  expr->type       = ZTEXPR_VALUE;
  expr->data.value = value;

  return expr;
}

ztast_expr_t *ztast_expr_from_scope(ztast_t *ast, ztast_scope_t *scope)
{
  ztast_expr_t *expr;

  assert(ast);
  assert(scope);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_expr_from_scope\n");
#endif

  expr = ZTAST_MALLOC(sizeof(*expr));
  if (expr == NULL)
    return NULL;

  expr->type       = ZTEXPR_SCOPE;
  expr->data.scope = scope;

  return expr;
}

ztast_expr_t *ztast_expr_from_intarray(ztast_t *ast, ztast_intarray_t *intarr)
{
  ztast_expr_t *expr;

  assert(ast);
  assert(intarr);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_expr_from_array\n");
#endif

  expr = ZTAST_MALLOC(sizeof(*expr));
  if (expr == NULL)
    return NULL;

  expr->type          = ZTEXPR_INTARRAY;
  expr->data.intarray = intarr;

  return expr;
}

ztast_expr_t *ztast_expr_from_scopearray(ztast_t            *ast,
                                         ztast_scopearray_t *scopearr)
{
  ztast_expr_t *expr;

  assert(ast);
  assert(scopearr);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_expr_from_array\n");
#endif

  expr = ZTAST_MALLOC(sizeof(*expr));
  if (expr == NULL)
    return NULL;

  expr->type            = ZTEXPR_SCOPEARRAY;
  expr->data.scopearray = scopearr;

  return expr;
}

ztast_value_t *ztast_value_from_integer(ztast_t *ast, int integer)
{
  ztast_value_t *val;

  assert(ast);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_value_from_integer\n");
#endif

  val = ZTAST_MALLOC(sizeof(*val));
  if (val == NULL)
    return NULL;

  val->type         = ZTVAL_INTEGER;
  val->data.integer = integer;

  return val;
}

ztast_value_t *ztast_value_from_decimal(ztast_t *ast, int decimal)
{
  ztast_value_t *val;

  assert(ast);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_value_from_decimal\n");
#endif

  val = ZTAST_MALLOC(sizeof(*val));
  if (val == NULL)
    return NULL;

  val->type         = ZTVAL_DECIMAL;
  val->data.decimal = decimal;

  return val;
}

ztast_value_t *ztast_value_nil(ztast_t *ast)
{
  ztast_value_t *val;

  assert(ast);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_value_nil\n");
#endif

  val = ZTAST_MALLOC(sizeof(*val));
  if (val == NULL)
    return NULL;

  val->type = ZTVAL_NIL;

  return val;
}

ztast_scope_t *ztast_scope(ztast_t *ast, ztast_statement_t *stmt)
{
  ztast_scope_t *scope;

  assert(ast);
  /* statement may be NULL */

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_scope\n");
#endif

  scope = ZTAST_MALLOC(sizeof(*scope));
  if (scope == NULL)
    return NULL;

  scope->statements = stmt;

  return scope;
}

ztast_intarray_t *ztast_intarray(ztast_t *ast, ztast_intarrayinner_t *inner)
{
  ztast_intarray_t *intarr;

  assert(ast);
  /* inner may be NULL */

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_intarray\n");
#endif

  intarr = ZTAST_MALLOC(sizeof(*intarr));
  if (intarr == NULL)
    return NULL;

  intarr->inner = inner;

  return intarr;
}

ztast_intarrayinner_t *ztast_intarrayinner_append(ztast_t               *ast,
                                                  ztast_intarrayinner_t *inner,
                                                  int                    val)
{
  assert(ast);
  /* inner may be NULL - which means allocate */

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_intarrayinner_append\n");
#endif

  if (inner == NULL)
  {
    inner = ZTAST_MALLOC(sizeof(*inner));
    if (inner == NULL)
      return NULL;

    inner->nused      = 0;
    inner->nallocated = 0;
    inner->ints       = NULL;
  }

  if (inner->nused == inner->nallocated)
  {
    int           newarrlen;
    unsigned int *newarr;

    /* This code pattern would use realloc() normally but our simple memory
     * manager doesn't currently feature it. */

    newarrlen = inner->nallocated < 8 ? 8 : inner->nallocated * 2;
    newarr = ZTAST_MALLOC(newarrlen * sizeof(*newarr));
    if (newarr == NULL)
      return NULL;

    memcpy(newarr, inner->ints, inner->nallocated * sizeof(*newarr));
    ZTAST_FREE(inner->ints);

    inner->nallocated = newarrlen;
    inner->ints       = newarr;
  }

  inner->ints[inner->nused++] = val;

  return inner;
}

ztast_scopearray_t *ztast_scopearray(ztast_t                 *ast,
                                     ztast_scopearrayinner_t *inner)
{
  ztast_scopearray_t *scopearr;

  assert(ast);
  /* inner may be NULL */

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_scopearray\n");
#endif

  scopearr = ZTAST_MALLOC(sizeof(*scopearr));
  if (scopearr == NULL)
    return NULL;

  scopearr->inner = inner;

  return scopearr;
}

ztast_scopearrayinner_t *ztast_scopearrayinner_append(ztast_t                 *ast,
                                                      ztast_scopearrayinner_t *inner,
                                                      ztast_scope_t           *scope)
{
  assert(ast);
  /* inner may be NULL - which means allocate */
  assert(scope);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_scopearrayinner_append\n");
#endif

  if (inner == NULL)
  {
    inner = ZTAST_MALLOC(sizeof(*inner));
    if (inner == NULL)
      return NULL;

    inner->nused      = 0;
    inner->nallocated = 0;
    inner->scopes     = NULL;
  }

  if (inner->nused == inner->nallocated)
  {
    int             newarrlen;
    ztast_scope_t **newarr;

    /* This code pattern would use realloc() normally but our simple memory
     * manager doesn't currently feature it. */

    newarrlen = inner->nallocated < 8 ? 8 : inner->nallocated * 2;
    newarr = ZTAST_MALLOC(newarrlen * sizeof(*newarr));
    if (newarr == NULL)
      return NULL;
 
    memcpy(newarr, inner->scopes, inner->nallocated * sizeof(*newarr));
    ZTAST_FREE(inner->scopes);

    inner->nallocated = newarrlen;
    inner->scopes     = newarr;
  }

  inner->scopes[inner->nused++] = scope;

  return inner;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
