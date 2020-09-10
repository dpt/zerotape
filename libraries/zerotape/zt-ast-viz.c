/* zt-ast-viz.c */

#include <assert.h>
#include <stdio.h>

#include "fortify.h"

#include "zt-ast.h"

#include "zt-ast-impl.h"

/* ----------------------------------------------------------------------- */

typedef struct ztast_viz_state
{
  FILE *file;
}
ztast_viz_state_t;

/* ----------------------------------------------------------------------- */

static ztresult_t ztast_viz_statements(ztast_viz_state_t       *state,
                                       const ztast_statement_t *statements,
                                       int                      depth);
static ztresult_t ztast_viz_expr(ztast_viz_state_t  *state,
                                 const ztast_expr_t *expr,
                                 int                 depth);

/* ----------------------------------------------------------------------- */

static ztresult_t ztast_viz_value(ztast_viz_state_t   *state,
                                  const ztast_value_t *value,
                                  int                  depth)
{
  int rc = ztresult_OK;

  switch (value->type)
  {
  case VAL_INTEGER:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{value|integer|%d}\"];\n", (void *) value, value->data.integer);
    break;

  case VAL_DECIMAL:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{value|decimal|%d}\"];\n", (void *) value, value->data.decimal);
    break;

  default:
    assert(0);
  }

  return rc;
}

static ztresult_t ztast_viz_id(ztast_viz_state_t *state,
                               const ztast_id_t  *id,
                               int                depth)
{
  (void) fprintf(state->file, "\t\"%p\" [label=\"{id|name|\'%s\'}\"];\n", (void *) id, id->name);

  return ztresult_OK;
}

static ztresult_t ztast_viz_array(ztast_viz_state_t   *state,
                                  const ztast_array_t *array,
                                  int                  depth)
{
  int                rc = ztresult_OK;
  int                i;
  ztast_arrayelem_t *elem;

  (void) fprintf(state->file, "\t\"%p\" [label=\"array\"];\n", (void *) array);
  (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) array, (void *) array->elems);

  /* turn -1 indices into runs of ascending indices */
  i = -1;
  for (elem = array->elems; elem; elem = elem->next)
  {
    i = (elem->index >= 0) ? elem->index : i + 1;

    (void) fprintf(state->file, "\t\"%p\" [label=\"index=%d\"];\n", (void *) elem, i);
    (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) elem, (void *) elem->expr);
    rc = ztast_viz_expr(state, elem->expr, depth + 1);
    if (rc)
      return rc;

    if (elem->next)
      (void) fprintf(state->file, "\t\"%p\":e -> \"%p\":w;\n", (void *) elem, (void *) elem->next);
  }

  return ztresult_OK;
}

static ztresult_t ztast_viz_expr(ztast_viz_state_t  *state,
                                 const ztast_expr_t *expr,
                                 int                 depth)
{
  int rc = ztresult_OK;

  switch (expr->type)
  {
  case EXPR_VALUE:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{expr|value}\"];\n", (void *) expr);
    (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) expr, (void *) expr->data.value);
    rc = ztast_viz_value(state, expr->data.value, depth + 1);
    break;

  case EXPR_ARRAY:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{expr|array}\"];\n", (void *) expr);
    (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) expr, (void *) expr->data.array);
    rc = ztast_viz_array(state, expr->data.array, depth + 1);
    break;

  case EXPR_SCOPE:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{expr|scope}\"];\n", (void *) expr);
    (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) expr, (void *) expr->data.scope->statements);
    rc = ztast_viz_statements(state, expr->data.scope->statements, depth + 1);
    break;
  }

  return rc;
}

static ztresult_t ztast_viz_statements(ztast_viz_state_t       *state,
                                       const ztast_statement_t *statements,
                                       int                      depth)
{
  int                      rc = ztresult_OK;
  const ztast_statement_t *stmt;

  assert(state);

  if (statements == NULL)
    return ztresult_OK; /* no statements */

  for (stmt = statements; stmt != NULL; stmt = stmt->next)
  {
    switch (stmt->type)
    {
    case STMT_ASSIGNMENT:
      (void) fprintf(state->file, "\t\"%p\" [label=\"{statement|assignment|{<lhs>lhs|<rhs>rhs}}\"];\n", (void *) stmt);
      (void) fprintf(state->file, "\t\"%p\":lhs -> \"%p\";\n", (void *) stmt, (void *) stmt->u.assignment->id);
      (void) ztast_viz_id(state, stmt->u.assignment->id, depth + 1);
      (void) fprintf(state->file, "\t\"%p\":rhs -> \"%p\";\n", (void *) stmt, (void *) stmt->u.assignment->expr);
      (void) ztast_viz_expr(state, stmt->u.assignment->expr, depth + 1);
      break;

    default:
      assert(0);
      break;
    }

    if (stmt->next)
      (void) fprintf(state->file, "\t\"%p\":e -> \"%p\":w;\n", (void *) stmt, (void *) stmt->next);
  }

  return rc;
}

static ztresult_t ztast_viz_program(ztast_viz_state_t     *state,
                                    const ztast_program_t *pgm,
                                    int                    depth)
{
  int rc;

  (void) fprintf(state->file, "\t\"%p\" [label=\"{AST|program}\"];\n", (void *) pgm);
  (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) pgm, (void *) pgm->statements);

  rc = ztast_viz_statements(state, pgm->statements, depth + 1);
  if (rc)
    return rc;

  return ztresult_OK;
}

ztresult_t ztast_show(ztast_t *ast, const char *filename)
{
  FILE             *file;
  ztast_viz_state_t state;
  int               rc;

  file = fopen(filename, "w");
  if (file == NULL)
    return ztresult_BAD_FOPEN;

  (void) fprintf(file, "digraph \"ast\"\n");
  (void) fprintf(file, "{\n");
  (void) fprintf(file, "\tnode [shape = Mrecord];\n");

  state.file = file;
  rc = ztast_viz_program(&state, ast->program, 0);

  (void) fprintf(file, "}\n");

  fclose(file);

  return rc;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
