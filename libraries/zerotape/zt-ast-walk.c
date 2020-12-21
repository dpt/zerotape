/* zt-ast-walk.c
 *
 * Test function to walk the AST to prove it's well-formed.
 */

#ifdef ZT_DEBUG

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "fortify/fortify.h"

#include "zt-ast.h"

/* ----------------------------------------------------------------------- */

typedef struct ztast_walk_state
{
  FILE *file;
  int   depth;
}
ztast_walk_state_t;

/* ----------------------------------------------------------------------- */

static void ztast_logf(const ztast_walk_state_t *state, const char *fmt, ...)
{
  int depth;
  va_list ap;

  depth = state->depth;
  while (depth--)
    fprintf(state->file, "--->");

  va_start(ap, fmt);
  vfprintf(state->file, fmt, ap);
  va_end(ap);
}

#define logf(ARGS) ztast_logf ARGS

/* ----------------------------------------------------------------------- */

static ztresult_t ztast_walk_statementlist(ztast_walk_state_t      *state,
                                           const ztast_statement_t *statements);

static ztresult_t ztast_walk_expr(ztast_walk_state_t *state,
                                  const ztast_expr_t *expr);

static ztresult_t ztast_walk_value(ztast_walk_state_t  *state,
                                   const ztast_value_t *value);

/* ----------------------------------------------------------------------- */

static ztresult_t ztast_walk_statementlist(ztast_walk_state_t      *state,
                                           const ztast_statement_t *statements)
{
  ztresult_t               rc;
  const ztast_statement_t *st;

  assert(state);

  if (statements == NULL)
    return ztresult_OK; /* no statements */

  logf((state, "ztast_walk_statementlist\n"));

  state->depth++;

  for (st = statements; st != NULL; st = st->next)
  {
    switch (st->type)
    {
    case ZTSTMT_ASSIGNMENT:
      /* FIXME: Bail if types don't match */
      logf((state, "assignment: name=%s\n", st->u.assignment->id->name));
      rc = ztast_walk_expr(state, st->u.assignment->expr);
      if (rc)
        return rc;
      break;

    default:
      assert(0);
      break;
    }
  }

  state->depth--;

  return ztresult_OK;
}

static ztresult_t ztast_walk_expr(ztast_walk_state_t *state,
                                  const ztast_expr_t *expr)
{
  int rc;

  assert(state);
  assert(expr);

  logf((state, "ztast_walk_expr\n"));

  state->depth++;

  switch (expr->type)
  {
  case ZTEXPR_VALUE:
    rc = ztast_walk_value(state, expr->data.value);
    if (rc)
      return rc;
    break;

  case ZTEXPR_SCOPE:
    rc = ztast_walk_statementlist(state, expr->data.scope->statements);
    if (rc)
      return rc;
    break;

  case ZTEXPR_INTARRAY:
    {
      ztast_intarrayinner_t *inner;

      inner = expr->data.intarray->inner;
      if (inner)
      {
        int i;

        for (i = 0; i < inner->nused; i++)
          logf((state, "index=%d value=%d\n", i, inner->ints[i]));
      }
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
          logf((state, "index=%d scope=%p\n", i, inner->scopes[i]));
      }
    }
    break;
  }

  state->depth--;

  return ztresult_OK;
}

static ztresult_t ztast_walk_value(ztast_walk_state_t  *state,
                                   const ztast_value_t *value)
{
  assert(state);
  assert(value);

  logf((state, "ztast_walk_value\n"));

  switch (value->type)
  {
  case ZTVAL_INTEGER:
    logf((state, "value, integer=%d\n", value->data.integer));
    break;

  case ZTVAL_DECIMAL:
    logf((state, "value, decimal=%d\n", value->data.decimal));
    break;

  case ZTVAL_NIL:
    logf((state, "nil\n"));
    break;

  default:
    assert(0);
  }

  return ztresult_OK;
}

ztresult_t ztast_walk(ztast_t *ast, void *opaque)
{
  int                rc;
  ztast_walk_state_t state;

  assert(ast);
  /* opaque may be NULL */

  state.file  = stdout;
  state.depth = 0;

  logf((&state, "ztast_walk\n"));

  state.depth++;
  /* There's no walk_program(), so start at statements. */
  rc = ztast_walk_statementlist(&state, ast->program->statements);
  state.depth--;

  return rc;
}

/* ----------------------------------------------------------------------- */

#endif /* ZT_DEBUG */

/* vim: set ts=8 sts=2 sw=2 et: */
