/* zt-ast-walk.c */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "fortify/fortify.h"

#include "zt-ast.h"

#include "zt-ast-impl.h"

/* ----------------------------------------------------------------------- */

typedef struct ztast_walk_state
{
  FILE *file;
  int   depth;
}
ztast_walk_state_t;

/* ----------------------------------------------------------------------- */

#ifdef ZT_DEBUG

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

#else

#define ztast_logf(...)

#endif

/* ----------------------------------------------------------------------- */

static ztresult_t ztast_walk_statements(ztast_walk_state_t *state,
                                  const ztast_statement_t  *statements);

/* ----------------------------------------------------------------------- */

static ztresult_t ztast_walk_value(ztast_walk_state_t *state,
                             const ztast_value_t      *value)
{
  switch (value->type)
  {
  case VAL_INTEGER:
    ztast_logf(state, "value, integer=%d\n", value->data.integer);
    break;

  case VAL_DECIMAL:
    ztast_logf(state, "value, decimal=%d\n", value->data.decimal);
    break;

  default:
    assert(0);
  }

  return ztresult_OK;
}

static ztresult_t ztast_walk_expr(ztast_walk_state_t *state,
                            const ztast_expr_t       *expr)
{
  int rc;

  ztast_logf(state, "ztast_walk_expr\n");

  state->depth++;

  switch (expr->type)
  {
  case EXPR_VALUE:
    rc = ztast_walk_value(state, expr->data.value);
    if (rc)
      return rc;
    break;

  case EXPR_ARRAY:
    {
      ztast_array_t     *array;
      int                i;
      ztast_arrayelem_t *elem;

      /* turn -1 indices into runs of ascending indices */
      /* TODO: Cope with array elems in the wrong order */
      array = expr->data.array;
      i = -1;
      for (elem = array->elems; elem; elem = elem->next)
      {
        i = (elem->index >= 0) ? elem->index : i + 1;

        ztast_logf(state, "arrayelem, index=%d\n", i, elem->expr);
        rc = ztast_walk_expr(state, elem->expr);
        if (rc)
          return rc;
      }
    }
    break;

  case EXPR_SCOPE:
    rc = ztast_walk_statements(state, expr->data.scope->statements);
    if (rc)
      return rc;
    break;
  }

  state->depth--;

  return ztresult_OK;
}

static ztresult_t ztast_walk_statements(ztast_walk_state_t *state,
                                  const ztast_statement_t  *statements)
{
  int rc = ztresult_OK;
  const ztast_statement_t *st;

  assert(state);

  if (statements == NULL)
    return ztresult_OK; /* no statements */

  ztast_logf(state, "ztast_walk_statements\n");

  state->depth++;

  for (st = statements; st != NULL; st = st->next)
  {
    switch (st->type)
    {
    case STMT_ASSIGNMENT:
      /* FIXME: Bail if types don't match */
      ztast_logf(state, "assign, name=%s\n", st->u.assignment->id->name);
      (void) ztast_walk_expr(state, st->u.assignment->expr);
      break;

    default:
      assert(0);
      break;
    }
  }

  state->depth--;
  return rc;
}

ztresult_t ztast_walk(ztast_t *ast, void *opaque)
{
  int                rc;
  ztast_walk_state_t state;

  assert(ast);

  state.file  = stdout;
  state.depth = 0;

  ztast_logf(&state, "ztast_walk\n");

  state.depth++;
  rc = ztast_walk_statements(&state, ast->program->statements);
  state.depth--;

  return rc;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
