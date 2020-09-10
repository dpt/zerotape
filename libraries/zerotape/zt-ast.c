/* zt-ast.c */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "fortify.h"

#include "zt-ast.h"

#include "zt-ast-impl.h"

/* TODO
 *
 * might want to store the lexerinfo structures in the AST at some point
 */

/* ----------------------------------------------------------------------- */

#define ZTAST_MALLOC(SZ) ast->mallocfn(SZ)
#define ZTAST_FREE(P)    ast->freefn(P)

/* ----------------------------------------------------------------------- */

ztast_t *ztast_create(ztast_mallocfn_t *mallocfn,
                      ztast_freefn_t   *freefn,
                      ztast_logfn_t    *logfn)
{
  ztast_t *ast;

  ast = mallocfn(sizeof(*ast));
  if (ast == NULL)
    return NULL;

  ast->program  = NULL;

  ast->mallocfn = mallocfn;
  ast->freefn   = freefn;
#ifdef ZTAST_LOG
  ast->logfn    = logfn;
#endif

  return ast;
}

typedef struct ztast_destroy_state
{
  void (*freefn)(void *);
}
ztast_destroy_state_t;

static void ztast_destroy_statements(ztast_destroy_state_t *state,
                                     ztast_statement_t     *statements);

static void ztast_destroy_value(ztast_destroy_state_t *state,
                                ztast_value_t         *value)
{
  state->freefn(value);
}

static void ztast_destroy_expr(ztast_destroy_state_t *state,
                               ztast_expr_t          *expr)
{
  switch (expr->type)
  {
  case EXPR_VALUE:
    ztast_destroy_value(state, expr->data.value);
    break;

  case EXPR_ARRAY:
    {
      ztast_array_t     *array;
      ztast_arrayelem_t *elem;
      ztast_arrayelem_t *next;

      /* turn -1 indices into runs of ascending indices */
      /* TODO: Cope with array elems in the wrong order */
      array = expr->data.array;
      for (elem = array->elems; elem; elem = next)
      {
        ztast_destroy_expr(state, elem->expr);
        next = elem->next;
        state->freefn(elem);
      }
      state->freefn(array);
    }
    break;

  case EXPR_SCOPE:
    ztast_destroy_statements(state, expr->data.scope->statements);
    state->freefn(expr->data.scope);
    break;
  }

  state->freefn(expr);
}

static void ztast_destroy_statements(ztast_destroy_state_t *state,
                                     ztast_statement_t     *statements)
{
  ztast_statement_t *st;
  ztast_statement_t *next;

  if (statements == NULL)
    return; /* no statements */

  for (st = statements; st != NULL; st = next)
  {
    switch (st->type)
    {
    case STMT_ASSIGNMENT:
      ztast_destroy_expr(state, st->u.assignment->expr);
      state->freefn(st->u.assignment->id);
      state->freefn(st->u.assignment);
      break;

    default:
      assert(0);
      break;
    }
    next = st->next;
    state->freefn(st);
  }
}

void ztast_destroy(ztast_t *ast)
{
  ztast_destroy_state_t state;

  if (ast == NULL)
    return;

  state.freefn = ast->freefn;

  ztast_destroy_statements(&state, ast->program->statements);

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
  stmt->type         = STMT_ASSIGNMENT;
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

  val->type         = VAL_INTEGER;
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

  val->type         = VAL_DECIMAL;
  val->data.decimal = decimal;

  return val;
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

  expr->type       = EXPR_VALUE;
  expr->data.value = value;

  return expr;
}

ztast_expr_t *ztast_expr_from_array(ztast_t *ast, ztast_array_t *array)
{
  ztast_expr_t *expr;

  assert(ast);
  assert(array);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_expr_from_array\n");
#endif

  expr = ZTAST_MALLOC(sizeof(*expr));
  if (expr == NULL)
    return NULL;

  expr->type       = EXPR_ARRAY;
  expr->data.array = array;

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

  expr->type       = EXPR_SCOPE;
  expr->data.scope = scope;

  return expr;
}

ztast_array_t *ztast_array(ztast_t *ast, ztast_arrayelem_t *elem)
{
  ztast_array_t *array;

  assert(ast);
  /* elem may be NULL */

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_array\n");
#endif

  array = ZTAST_MALLOC(sizeof(*array));
  if (array == NULL)
    return NULL;

  array->elems = elem;

  return array;
}

ztast_arrayelem_t *ztast_arrayelem(ztast_t            *ast,
                                   ztast_arrayindex_t  index,
                                   ztast_expr_t       *expr)
{
  ztast_arrayelem_t *arrayelem;

  assert(ast);
  assert(expr);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_arrayelem\n");
#endif

  arrayelem = ZTAST_MALLOC(sizeof(*arrayelem));
  if (arrayelem == NULL)
    return NULL;

  arrayelem->next  = NULL;
  arrayelem->index = index;
  arrayelem->expr  = expr;

  return arrayelem;
}

void ztast_arrayelem_append(ztast_t           *ast,
                            ztast_arrayelem_t *arrayelemlist,
                            ztast_arrayelem_t *appendee)
{
  ztast_arrayelem_t *elem;

  assert(ast);
  assert(arrayelemlist);
  assert(appendee);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_arrayelem_append %p to %p", appendee, arrayelemlist);
#endif

  for (elem = arrayelemlist; elem->next != NULL; elem = elem->next)
    ;
  elem->next = appendee;

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn(" -> %p\n", elem);
#endif
}

ztast_scope_t *ztast_scope(ztast_t *ast, ztast_statement_t *statement)
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

  scope->statements = statement;

  return scope;
}

void ast_scope_append(ztast_t           *ast,
                      ztast_scope_t     *scope,
                      ztast_statement_t *appendee)
{
  ztast_statement_t *stmt;

  assert(ast);
  assert(scope);
  assert(appendee);

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn("ztast_scope_append %p to %p", appendee, scope);
#endif

  for (stmt = scope->statements; stmt->next != NULL; stmt = stmt->next)
    ;
  stmt->next = appendee;

#ifdef ZTAST_LOG
  if (ast->logfn)
    ast->logfn(" -> %p\n", stmt);
#endif
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
