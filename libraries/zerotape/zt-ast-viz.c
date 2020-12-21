/* zt-ast-viz.c */

#include <assert.h>
#include <stdio.h>

#include "fortify/fortify.h"

#include "zt-ast.h"

/* ----------------------------------------------------------------------- */

typedef struct ztast_viz_state
{
  FILE *file;
}
ztast_viz_state_t;

/* ----------------------------------------------------------------------- */

static ztresult_t ztast_viz_statementlist(ztast_viz_state_t       *state,
                                          const ztast_statement_t *stmts,
                                          int                      depth);

static ztresult_t ztast_viz_assignment(ztast_viz_state_t        *state,
                                       const ztast_assignment_t *ass,
                                       int                       depth);

static ztresult_t ztast_viz_id(ztast_viz_state_t *state,
                               const ztast_id_t  *id,
                               int                depth);

static ztresult_t ztast_viz_expr(ztast_viz_state_t  *state,
                                 const ztast_expr_t *expr,
                                 int                 depth);

static ztresult_t ztast_viz_value(ztast_viz_state_t   *state,
                                  const ztast_value_t *value,
                                  int                  depth);

static ztresult_t ztast_viz_scope(ztast_viz_state_t   *state,
                                  const ztast_scope_t *value,
                                  int                  depth);

static ztresult_t ztast_viz_intarray(ztast_viz_state_t      *state,
                                     const ztast_intarray_t *intarr,
                                     int                     depth);

static ztresult_t ztast_viz_scopearray(ztast_viz_state_t        *state,
                                       const ztast_scopearray_t *scopearr,
                                       int                       depth);

/* ----------------------------------------------------------------------- */

static ztresult_t ztast_viz_program(ztast_viz_state_t     *state,
                                    const ztast_program_t *prog,
                                    int                    depth)
{
  (void) fprintf(state->file, "\t\"%p\" [label=\"{AST|program}\"];\n", (void *) prog);
  (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) prog, (void *) prog->statements);

  return ztast_viz_statementlist(state, prog->statements, depth + 1);
}

static ztresult_t ztast_viz_statementlist(ztast_viz_state_t       *state,
                                          const ztast_statement_t *stmtlist,
                                          int                      depth)
{
  ztresult_t               rc;
  const ztast_statement_t *stmt;

  assert(state);

  if (stmtlist == NULL)
    return ztresult_OK; /* no statements */

  for (stmt = stmtlist; stmt != NULL; stmt = stmt->next)
  {
    switch (stmt->type)
    {
    case ZTSTMT_ASSIGNMENT:
      (void) fprintf(state->file, "\t\"%p\" [label=\"{statement|<ass>assignment}\"];\n", (void *) stmt);
      (void) fprintf(state->file, "\t\"%p\":ass -> \"%p\";\n", (void *) stmt, (void *) stmt->u.assignment);
      rc = ztast_viz_assignment(state, stmt->u.assignment, depth + 1);
      if (rc)
        return rc;
      break;

    default:
      assert(0);
      break;
    }

    if (stmt->next)
      (void) fprintf(state->file, "\t\"%p\":e -> \"%p\":w;\n", (void *) stmt, (void *) stmt->next);
  }

  return ztresult_OK;
}

static ztresult_t ztast_viz_assignment(ztast_viz_state_t        *state,
                                       const ztast_assignment_t *assmt,
                                       int                       depth)
{
  ztresult_t rc;

  (void) fprintf(state->file, "\t\"%p\" [label=\"{assignment|{<lhs>lhs|<rhs>rhs}}\"];\n", (void *) assmt);
  (void) fprintf(state->file, "\t\"%p\":lhs -> \"%p\";\n", (void *) assmt, (void *) assmt->id);

  rc = ztast_viz_id(state, assmt->id, depth + 1);
  if (rc)
    return rc;

  (void) fprintf(state->file, "\t\"%p\":rhs -> \"%p\";\n", (void *) assmt, (void *) assmt->expr);

  rc = ztast_viz_expr(state, assmt->expr, depth + 1);
  if (rc)
    return rc;

  return ztresult_OK;
}

static ztresult_t ztast_viz_id(ztast_viz_state_t *state,
                               const ztast_id_t  *id,
                               int                depth)
{
  (void) fprintf(state->file, "\t\"%p\" [label=\"{id|name|\'%s\'}\"];\n", (void *) id, id->name);

  return ztresult_OK;
}

static ztresult_t ztast_viz_expr(ztast_viz_state_t  *state,
                                 const ztast_expr_t *expr,
                                 int                 depth)
{
  ztresult_t rc = ztresult_OK;

  switch (expr->type)
  {
  case ZTEXPR_VALUE:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{expr|value}\"];\n", (void *) expr);
    (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) expr, (void *) expr->data.value);
    rc = ztast_viz_value(state, expr->data.value, depth + 1);
    break;

  case ZTEXPR_SCOPE:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{expr|scope}\"];\n", (void *) expr);
    (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) expr, (void *) expr->data.scope);
    rc = ztast_viz_scope(state, expr->data.scope, depth + 1);
    break;

  case ZTEXPR_INTARRAY:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{expr|intarray}\"];\n", (void *) expr);
    (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) expr, (void *) expr->data.intarray);
    rc = ztast_viz_intarray(state, expr->data.intarray, depth + 1);
    break;

  case ZTEXPR_SCOPEARRAY:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{expr|scopearray}\"];\n", (void *) expr);
    (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) expr, (void *) expr->data.scopearray);
    rc = ztast_viz_scopearray(state, expr->data.scopearray, depth + 1);
    break;

  default:
    assert(0);
  }

  return rc;
}

static ztresult_t ztast_viz_value(ztast_viz_state_t   *state,
                                  const ztast_value_t *val,
                                  int                  depth)
{
  ztresult_t rc = ztresult_OK;

  switch (val->type)
  {
  case ZTVAL_INTEGER:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{value|integer|%d}\"];\n", (void *) val, val->data.integer);
    break;

  case ZTVAL_DECIMAL:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{value|decimal|%d}\"];\n", (void *) val, val->data.decimal);
    break;

  case ZTVAL_NIL:
    (void) fprintf(state->file, "\t\"%p\" [label=\"{value|nil}\"];\n", (void *) val);
    break;

  default:
    assert(0);
  }

  return rc;
}

static ztresult_t ztast_viz_scope(ztast_viz_state_t   *state,
                                  const ztast_scope_t *scope,
                                  int                  depth)
{
  (void) fprintf(state->file, "\t\"%p\" [label=\"{scope|<stt>statements}\"];\n", (void *) scope);
  if (scope->statements)
    (void) fprintf(state->file, "\t\"%p\":stt -> \"%p\";\n", (void *) scope, (void *) scope->statements);

  return ztast_viz_statementlist(state, scope->statements, depth + 1);
}

static ztresult_t ztast_viz_intarray(ztast_viz_state_t      *state,
                                     const ztast_intarray_t *intarr,
                                     int                     depth)
{
  (void) fprintf(state->file, "\t\"%p\" [label=\"intarray\"];\n", (void *) intarr);

  return ztresult_OK;
}

static ztresult_t ztast_viz_scopearray(ztast_viz_state_t        *state,
                                       const ztast_scopearray_t *scopearr,
                                       int                       depth)
{
  ztast_scopearrayinner_t *inner;
  int                      i;

  (void) fprintf(state->file, "\t\"%p\" [label=\"scopearray\"];\n", (void *) scopearr);

  inner = scopearr->inner;
  if (inner == NULL)
    return ztresult_OK;

  for (i = 0; i < scopearr->inner->nused; i++)
  {
    ztresult_t rc;

    (void) fprintf(state->file, "\t\"%p\" -> \"%p\";\n", (void *) scopearr, (void *) scopearr->inner->scopes[i]);

    rc = ztast_viz_scope(state, scopearr->inner->scopes[i], depth + 1);
    if (rc)
      return rc;
  }

  return ztresult_OK;
}

ztresult_t ztast_show(ztast_t *ast, const char *filename)
{
  ztresult_t        rc;
  FILE             *file;
  ztast_viz_state_t state;

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
