/* zt-run.c */

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zerotape/zerotape.h"

#include "zt-parser.h"
#include "zt-ast.h"

#include "zt-run.h"

/* ----------------------------------------------------------------------- */

#ifdef ZT_DEBUG
#define logf(ARGS) printf ARGS
#else
#define logf(ARGS)
#endif

/* ----------------------------------------------------------------------- */

/** Run the supplied list of statements against the given state structure
 * defined in 'meta'. */
static ztresult_t zt_run_statements(const ztast_statement_t *statements,
                                    const ztstruct_t        *meta,
                                    const ztregion_t        *regions,
                                    int                      nregions,
                                    ztloader_t             **loaders,
                                    int                      nloaders,
                                    void                    *structure,
                                    char                   **syntax_error);

/* ----------------------------------------------------------------------- */

/** Enumeration of possible syntax errors. */
typedef enum ztsyntaxerr
{
  ztsyntx_NEED_ARRAY,
  ztsyntx_NEED_DECIMAL,
  ztsyntx_NEED_INTEGER,
  ztsyntx_NEED_SCOPE,
  ztsyntx_NEED_VALUE,
  ztsyntx_UNEXPECTED_VALUE_TYPE,
  ztsyntx_UNKNOWN_FIELD,
  ztsyntx_UNKNOWN_REGION,
  ztsyntx_UNSUPPORTED,
  ztsyntx_VALUE_RANGE,
  ztsyntx__LIMIT
}
ztsyntaxerr_t;

static const char *zt_syntaxstring(ztsyntaxerr_t e)
{
  static const char *tab[] =
  {
    /* ztsyntx_NEED_ARRAY */ "array required",
    /* ztsyntx_NEED_DECIMAL */ "decimal type required",
    /* ztsyntx_NEED_INTEGER */ "integer type required",
    /* ztsyntx_NEED_SCOPE */ "scope required",
    /* ztsyntx_NEED_VALUE */ "value type required", /* e.g. an array or scope received */
    /* ztsyntx_UNEXPECTED_VALUE_TYPE */ "unexpected value type",
    /* ztsyntx_UNKNOWN_FIELD */ "unknown field",
    /* ztsyntx_UNKNOWN_REGION */ "unknown region",
    /* ztsyntx_UNSUPPORTED */ "unsupported",
    /* ztsyntx_VALUE_RANGE */ "value out of range"
  };
  if (e < ztsyntx__LIMIT)
    return tab[e];
  else
    return "unknown error";
}

/** Build and return a syntax error message. */
static ztresult_t zt_mksyntax(char **syntax_error, ztsyntaxerr_t e)
{
  *syntax_error = (char *) zt_syntaxstring(e);
  return ztresult_SYNTAX_ERROR;
}

/** Point to the specified value in state. */
#define PVAL(STATE, OFFSET) ((void *)((char *) STATE + OFFSET))

/** Handle a single integer field, or array of. */
#define DO_INLINE(TYPE, MAX)                                                 \
  do {                                                                       \
    if (field->nelems == 1) { /* expecting a single element */               \
      const ztast_expr_t  *expr;                                             \
      const ztast_value_t *value;                                            \
      TYPE                *rawvalue;                                         \
      unsigned int         integer;                                          \
                                                                             \
      expr = assignment->expr;                                               \
      if (expr->type != ZTEXPR_VALUE)                                        \
        return zt_mksyntax(syntax_error, ztsyntx_NEED_VALUE);                \
      value = expr->data.value;                                              \
      if (value->type != ZTVAL_INTEGER)                                      \
        return zt_mksyntax(syntax_error, ztsyntx_NEED_INTEGER);              \
      integer = value->data.integer;                                         \
      if (integer < 0 || integer > MAX)                                      \
        return zt_mksyntax(syntax_error, ztsyntx_VALUE_RANGE);               \
                                                                             \
      rawvalue = PVAL(structure, field->offset);                             \
      *rawvalue = integer;                                                   \
    } else { /* expecting an array */                                        \
      const ztast_expr_t      *arrayexpr;                                    \
      int                      index;                                        \
      TYPE                    *rawarray;                                     \
      const ztast_arrayelem_t *elem;                                         \
                                                                             \
      arrayexpr = assignment->expr;                                          \
      if (arrayexpr->type != ZTEXPR_ARRAY)                                   \
        return zt_mksyntax(syntax_error, ztsyntx_NEED_ARRAY);                \
                                                                             \
      index    = -1;                                                         \
      rawarray = PVAL(structure, field->offset);                             \
      for (elem = arrayexpr->data.array->elems; elem; elem = elem->next) {   \
        ztast_expr_t  *expr;                                                 \
        ztast_value_t *value;                                                \
        unsigned int   integer;                                              \
                                                                             \
        /* Elements without a specified position will have an index of -1 */ \
        index = (elem->index >= 0) ? elem->index : index + 1;                \
                                                                             \
        expr = elem->expr;                                                   \
        if (expr->type != ZTEXPR_VALUE)                                      \
          return zt_mksyntax(syntax_error, ztsyntx_NEED_VALUE);              \
        value = expr->data.value;                                            \
        if (value->type != ZTVAL_INTEGER)                                    \
          return zt_mksyntax(syntax_error, ztsyntx_NEED_INTEGER);            \
        integer = value->data.integer;                                       \
        if (integer < 0 || integer > MAX)                                    \
          return zt_mksyntax(syntax_error, ztsyntx_VALUE_RANGE);             \
                                                                             \
        rawarray[index] = integer;                                           \
        logf(("%d=%d,", index, integer));                                    \
      }                                                                      \
      logf(("\n"));                                                          \
    }                                                                        \
  } while (0)

/** Handle a pointer to single integer field, or array of. */
#define DO_POINTER(TYPE, MAX)                                                \
  do {                                                                       \
    if (field->nelems == 1) { /* expecting a single element */               \
      const ztast_expr_t  *valueexpr;                                        \
      const ztast_value_t *value;                                            \
      TYPE               **prawvalue;                                        \
      TYPE                *rawvalue;                                         \
      unsigned int         integer;                                          \
                                                                             \
      valueexpr = assignment->expr;                                          \
      if (valueexpr->type != ZTEXPR_VALUE)                                   \
        return zt_mksyntax(syntax_error, ztsyntx_NEED_VALUE);                \
      value = valueexpr->data.value;                                         \
      if (value->type != ZTVAL_INTEGER)                                      \
        return zt_mksyntax(syntax_error, ztsyntx_NEED_INTEGER);              \
      integer = value->data.integer;                                         \
      if (integer < 0 || integer > MAX)                                      \
        return zt_mksyntax(syntax_error, ztsyntx_VALUE_RANGE);               \
                                                                             \
      prawvalue = PVAL(structure, field->offset);                            \
      rawvalue  = *prawvalue;                                                \
      *rawvalue = integer;                                                   \
    } else { /* expecting an array */                                        \
      const ztast_expr_t      *arrayexpr;                                    \
      int                      index;                                        \
      TYPE                   **prawarray; /* fix constness here */           \
      TYPE                    *rawarray;                                     \
      const ztast_arrayelem_t *elem;                                         \
                                                                             \
      arrayexpr = assignment->expr;                                          \
      if (arrayexpr->type != ZTEXPR_ARRAY)                                   \
        return zt_mksyntax(syntax_error, ztsyntx_NEED_ARRAY);                \
                                                                             \
      index     = -1;                                                        \
      prawarray = PVAL(structure, field->offset);                            \
      rawarray  = *prawarray;                                                \
      for (elem = arrayexpr->data.array->elems; elem; elem = elem->next) {   \
        ztast_expr_t  *expr;                                                 \
        ztast_value_t *value;                                                \
        unsigned int   integer;                                              \
                                                                             \
        /* Elements without a specified position will have an index of -1 */ \
        index = (elem->index >= 0) ? elem->index : index + 1;                \
                                                                             \
        expr = elem->expr;                                                   \
        if (expr->type != ZTEXPR_VALUE)                                      \
          return zt_mksyntax(syntax_error, ztsyntx_NEED_VALUE);              \
        value = expr->data.value;                                            \
        if (value->type != ZTVAL_INTEGER)                                    \
          return zt_mksyntax(syntax_error, ztsyntx_NEED_INTEGER);            \
        integer = value->data.integer;                                       \
        if (integer < 0 || integer > MAX)                                    \
          return zt_mksyntax(syntax_error, ztsyntx_VALUE_RANGE);             \
                                                                             \
        rawarray[index] = integer;                                           \
        logf(("%d=%d,", index, integer));                                    \
      }                                                                      \
      logf(("\n"));                                                          \
    }                                                                        \
  } while (0)

/**
 * Execute an assignment statement.
 *
 * \param assignment assignment statement to execute
 * \param meta description of 'structure'
 * \param regions runtime heap array specs
 * \param nregions number of heap array specs
 * \param loaders array of loader functions - one per custom ID
 * \param nloaders number of loader functions
 * \param structure structure to populate
 * \param syntax_error error message if (result != ztresult_OK), else NULL
 */
static ztresult_t zt_do_assignment(const ztast_assignment_t *assignment,
                                   const ztstruct_t         *meta,
                                   const ztregion_t         *regions,
                                   int                       nregions,
                                   ztloader_t              **loaders,
                                   int                       nloaders,
                                   void                     *structure,
                                   char                    **syntax_error)
{
  const char      *name;
  int              f;
  const ztfield_t *field;

  *syntax_error = NULL;

  name = assignment->id->name;
  logf(("assignment to field '%s'\n", name));
  for (f = 0; f < meta->nfields; f++)
    if (strcmp(meta->fields[f].name, name) == 0)
      break;
  if (f == meta->nfields)
    zt_mksyntax(syntax_error, ztsyntx_UNKNOWN_FIELD);

  field = &meta->fields[f];
  assert(field->nelems >= 1);
  switch (field->type)
  {
  case zttype_uchar:
    DO_INLINE(ztuchar_t, UCHAR_MAX);
    break;
  case zttype_ucharptr:
    DO_POINTER(ztuchar_t, UCHAR_MAX);
    break;
  case zttype_ushort:
    DO_INLINE(ztushort_t, USHRT_MAX);
    break;
  case zttype_ushortptr:
    DO_POINTER(ztushort_t, USHRT_MAX);
    break;
  case zttype_uint:
    DO_INLINE(ztuint_t, UINT_MAX);
    break;
  case zttype_uintptr:
    DO_POINTER(ztuint_t, UINT_MAX);
    break;

  case zttype_struct:
    if (field->nelems == 1) /* expecting a single element */
    {
      ztresult_t          rc;
      const ztast_expr_t *scopeexpr;
      void               *rawstruct;

      scopeexpr = assignment->expr;
      if (scopeexpr->type != ZTEXPR_SCOPE)
        return zt_mksyntax(syntax_error, ztsyntx_NEED_SCOPE);

      rawstruct = PVAL(structure, field->offset);

      rc = zt_run_statements(scopeexpr->data.scope->statements,
                             field->metadata,
                             regions,
                             nregions,
                             loaders,
                             nloaders,
                             rawstruct,
                             syntax_error);
      if (rc)
        return rc;
    }
    else /* expecting an array */
    {
      const size_t             elsz = field->size; /* field->size is sizeof(struct) */
      ztresult_t               rc;
      const ztast_expr_t      *arrayexpr;
      int                      index;
      void                    *rawstruct;
      const ztast_arrayelem_t *elem;

      arrayexpr = assignment->expr;
      if (arrayexpr->type != ZTEXPR_ARRAY)
        return zt_mksyntax(syntax_error, ztsyntx_NEED_ARRAY);

      index = -1;

      rawstruct = PVAL(structure, field->offset);

      for (elem = arrayexpr->data.array->elems; elem; elem = elem->next)
      {
        const ztast_expr_t *expr;

        /* Elements without a specified position will have an index of -1 */
        index = (elem->index >= 0) ? elem->index : index + 1;

        expr = elem->expr;
        if (expr->type != ZTEXPR_SCOPE)
          return zt_mksyntax(syntax_error, ztsyntx_NEED_SCOPE);

        rc = zt_run_statements(expr->data.scope->statements,
                               field->metadata,
                               regions,
                               nregions,
                               loaders,
                               nloaders,
                      (char *) rawstruct + index * elsz,
                               syntax_error);
        if (rc)
          return rc;
      }
    }
    break;

  case zttype_structptr:
    if (field->nelems == 1) /* expecting a single element */
    {
      ztresult_t          rc;
      const ztast_expr_t *scopeexpr;
      void              **prawstruct;
      void               *rawstruct;

      scopeexpr = assignment->expr;
      if (scopeexpr->type != ZTEXPR_SCOPE)
        return zt_mksyntax(syntax_error, ztsyntx_NEED_SCOPE);

      prawstruct = PVAL(structure, field->offset);
      rawstruct = *prawstruct;

      rc = zt_run_statements(scopeexpr->data.scope->statements,
                             field->metadata,
                             regions,
                             nregions,
                             loaders,
                             nloaders,
                             rawstruct,
                             syntax_error);
      if (rc)
        return rc;
    }
    else /* expecting an array */
    {
      const size_t             elsz = field->size; /* field->size is sizeof(struct) */
      ztresult_t               rc;
      const ztast_expr_t      *arrayexpr;
      int                      index;
      void                   **prawstruct;
      void                    *rawstruct;
      const ztast_arrayelem_t *elem;

      arrayexpr = assignment->expr;
      if (arrayexpr->type != ZTEXPR_ARRAY)
        return zt_mksyntax(syntax_error, ztsyntx_NEED_ARRAY);

      index = -1;

      prawstruct = PVAL(structure, field->offset);
      rawstruct = *prawstruct;

      for (elem = arrayexpr->data.array->elems; elem; elem = elem->next)
      {
        const ztast_expr_t *expr;

        /* Elements without a specified position will have an index of -1 */
        index = (elem->index >= 0) ? elem->index : index + 1;

        expr = elem->expr;
        if (expr->type != ZTEXPR_SCOPE)
          return zt_mksyntax(syntax_error, ztsyntx_NEED_SCOPE);

        rc = zt_run_statements(expr->data.scope->statements,
                               field->metadata,
                               regions,
                               nregions,
                               loaders,
                               nloaders,
                      (char *) rawstruct + index * elsz,
                               syntax_error);
        if (rc)
          return rc;
      }
    }
    break;

  case zttype_staticarrayidx:
    assert(field->array);
    assert(field->regionid == ZT_NO_REGIONID);
    if (field->nelems == 1) /* expecting a single element */
    {
      const size_t        elsz = field->array->length / field->array->nelems; /* field->array->length is total size of array */
      const ztast_expr_t *assignmentexpr;
      void              **prawvalue;
      int                 index;

      assignmentexpr = assignment->expr;
      if (assignmentexpr->type != ZTEXPR_VALUE)
        return zt_mksyntax(syntax_error, ztsyntx_NEED_VALUE);

      prawvalue = PVAL(structure, field->offset);

      switch (assignmentexpr->data.value->type)
      {
      case ZTVAL_INTEGER:
        index = assignmentexpr->data.value->data.integer;
        if (index < 0 || index >= field->array->nelems)
          return zt_mksyntax(syntax_error, ztsyntx_VALUE_RANGE);

        *prawvalue = (char *) field->array->base + index * elsz;
        break;

      case ZTVAL_NIL:
        *prawvalue = NULL;
        break;

      default:
        return zt_mksyntax(syntax_error, ztsyntx_UNEXPECTED_VALUE_TYPE);
      }
    }
    else /* expecting an array */
    {
      return zt_mksyntax(syntax_error, ztsyntx_UNSUPPORTED);
    }
    break;

  case zttype_arrayidx:
    {
      int              r;
      const ztarray_t *array;
      size_t           elsz;

      assert(field->array == ZT_NO_ARRAY);
      assert(field->regionid);

      for (r = 0; r < nregions; r++)
        if (field->regionid == regions[r].id)
          break;
      if (r == nregions)
        return zt_mksyntax(syntax_error, ztsyntx_UNKNOWN_REGION);

      array = &regions[r].spec;
      elsz  = array->length / array->nelems;

      if (field->nelems == 1) /* expecting a single element */
      {
        const ztast_expr_t *assignmentexpr;
        void              **prawvalue;
        int                 index;

        assignmentexpr = assignment->expr;
        if (assignmentexpr->type != ZTEXPR_VALUE)
          return zt_mksyntax(syntax_error, ztsyntx_NEED_VALUE);

        prawvalue = PVAL(structure, field->offset);

        switch (assignmentexpr->data.value->type)
        {
        case ZTVAL_INTEGER:
          index = assignmentexpr->data.value->data.integer;
          if (index < 0 || index >= array->nelems)
            return zt_mksyntax(syntax_error, ztsyntx_VALUE_RANGE);

          *prawvalue = (char *) array->base + index * elsz;
          break;

        case ZTVAL_NIL:
          *prawvalue = NULL;
          break;

        default:
          return zt_mksyntax(syntax_error, ztsyntx_UNEXPECTED_VALUE_TYPE);
        }
      }
      else /* expecting an array */
      {
        return zt_mksyntax(syntax_error, ztsyntx_UNSUPPORTED);
      }
      break;
    }

  case zttype_version:
    if (field->nelems == 1) /* expecting a single element */
    {
      const ztast_expr_t *assignmentexpr;
      int                 decimal;
      int                *prawvalue;

      assignmentexpr = assignment->expr;
      if (assignmentexpr->type != ZTEXPR_VALUE)
        return zt_mksyntax(syntax_error, ztsyntx_NEED_VALUE);
      if (assignmentexpr->data.value->type != ZTVAL_DECIMAL)
        return zt_mksyntax(syntax_error, ztsyntx_NEED_DECIMAL);
      decimal = assignmentexpr->data.value->data.decimal;
      if (decimal < 0 || decimal > 999)
        return zt_mksyntax(syntax_error, ztsyntx_VALUE_RANGE);

      prawvalue = PVAL(structure, field->offset);
      *prawvalue = decimal;
    }
    else /* expecting an array */
    {
      return zt_mksyntax(syntax_error, ztsyntx_UNSUPPORTED);
    }
    break;

  case zttype_custom:
    if (field->nelems == 1) /* expecting a single element */
    {
      const ztast_expr_t *assignmentexpr;
      void               *prawvalue;

      assignmentexpr = assignment->expr;
      prawvalue      = PVAL(structure, field->offset);
      return loaders[field->typeidx](assignmentexpr, prawvalue, syntax_error);
    }
    else /* expecting an array */
    {
      return zt_mksyntax(syntax_error, ztsyntx_UNSUPPORTED);
    }
    break;
  }

  return ztresult_OK;
}

/**
 * Execute the given statements.
 *
 * \param statements AST
 * \param meta description of 'structure'
 * \param regions runtime heap array specs
 * \param nregions number of heap array specs
 * \param loaders array of loader functions - one per custom ID
 * \param nloaders number of loader functions
 * \param structure structure to populate
 * \param syntax_error error message, or NULL if none
 */
static ztresult_t zt_run_statements(const ztast_statement_t *statements,
                                    const ztstruct_t        *meta,
                                    const ztregion_t        *regions,
                                    int                      nregions,
                                    ztloader_t             **loaders,
                                    int                      nloaders,
                                    void                    *structure,
                                    char                   **syntax_error)
{
  ztresult_t               rc;
  const ztast_statement_t *statement;

  for (statement = statements; statement; statement = statement->next)
  {
    switch (statement->type)
    {
    case ZTSTMT_ASSIGNMENT:
      rc = zt_do_assignment(statement->u.assignment,
                            meta,
                            regions,
                            nregions,
                            loaders,
                            nloaders,
                            structure,
                            syntax_error);
      if (rc)
        return rc;
      break;

    default:
      return zt_mksyntax(syntax_error, ztsyntx_UNSUPPORTED);
    }
  }

  return ztresult_OK;
}

ztresult_t zt_run_program(const ztast_t     *ast,
                          const ztstruct_t  *metastruct,
                          const ztregion_t  *regions,
                          int                nregions,
                          ztloader_t       **loaders,
                          int                nloaders,
                          void              *structure,
                          char             **syntax_error)
{
  if (ast->program == NULL)
    return ztresult_NO_PROGRAM;

  return zt_run_statements(ast->program->statements,
                           metastruct,
                           regions,
                           nregions,
                           loaders,
                           nloaders,
                           structure,
                           syntax_error);
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
