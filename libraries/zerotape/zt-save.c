/* zt-save.c */

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zerotape/zerotape.h"

#include "zt-walk.h"

/* ----------------------------------------------------------------------- */

/* printf formatting token for outputting integers. */
#ifdef ZT_USE_HEX
#define FMT "$%X" /* hex */
#else
#define FMT "%u"
#endif

/* ----------------------------------------------------------------------- */

typedef struct savestack_entry
{
  enum { Struct, Array } container;
  int nelems;
  int index;
}
savestack_entry_t;

typedef struct save_stack
{
  savestack_entry_t *sp;        /* stack pointer. points at next empty entry */
  savestack_entry_t *stack;     /* stack of entries */
  int                allocated; /* number of entries allocated */
}
save_stack_t;

static int savestack_depth(const save_stack_t *stack)
{
  return (int)(stack->sp - stack->stack);
}

static int savestack_empty(const save_stack_t *stack)
{
  return savestack_depth(stack) == 0;
}

static savestack_entry_t *savestack_top(const save_stack_t *stack)
{
  if (savestack_empty(stack))
    return NULL;

  return stack->sp - 1;
}

static ztresult_t savestack_push(save_stack_t      *stack,
                           const savestack_entry_t *entry)
{
  const int InitialStackSize      = 4; /* initial size in entries */
  const int StackGrowthMultiplier = 2; /* doubling strategy */

  if (stack->sp == stack->stack + stack->allocated)
  {
    int                newallocated;
    savestack_entry_t *newstack;

    newallocated = stack->allocated * StackGrowthMultiplier;
    if (newallocated < InitialStackSize)
      newallocated = InitialStackSize;

    newstack = realloc(stack->stack, newallocated * sizeof(*newstack)); /* FIXME: hoist to a memory manager */
    if (newstack == NULL)
      return ztresult_OOM;

    stack->sp        = newstack + (stack->sp - stack->stack); /* adjust sp */
    stack->stack     = newstack;
    stack->allocated = newallocated;
  }

  *stack->sp++ = *entry;

  return ztresult_OK;
}

/* this is "pop and discard" really */
static void savestack_pop(save_stack_t *stack)
{
  if (savestack_empty(stack))
    return;

  stack->sp--;
}

static void savestack_setup(save_stack_t *stack)
{
  stack->sp        = NULL;
  stack->stack     = NULL;
  stack->allocated = 0;
}

static void savestack_destroy(save_stack_t *stack)
{
  if (stack == NULL)
    return;

  free(stack->stack);
}

/* ----------------------------------------------------------------------- */

typedef struct savestate
{
  FILE              *f;
  int                indent_is_due;
  int                depth;
  save_stack_t       stack;
  ztsaver_t        **savers;
  int                nsavers;
}
savestate_t;

static void indent(savestate_t *state)
{
  state->depth++;
}

static void outdent(savestate_t *state)
{
  state->depth--;
}

static void emitf(savestate_t *state, const char *fmt, ...)
{
  va_list ap;
  size_t  fmtlen;

  if (state->indent_is_due)
  {
    int depth;
    int i;

    depth = state->depth;
    for (i = 0; i < depth; i++)
    {
#ifdef ZT_DEBUG
      printf("  ");
#endif
      fprintf(state->f, "  ");
    }
  }

#ifdef ZT_DEBUG
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
#endif

  va_start(ap, fmt);
  vfprintf(state->f, fmt, ap);
  va_end(ap);

  fmtlen = strlen(fmt);
  state->indent_is_due = (fmt[fmtlen - 1] == '\n'); /* weak newline detection */
}

/* ----------------------------------------------------------------------- */

#define DUMP(TYPE)                                                     \
  do {                                                                 \
    if (nelems == 1) { /* singletons are rendered as: x = $0; */       \
      emitf(state, "%s = " FMT ";\n", name, *pvalue);                  \
    } else { /* arrays are rendered as: x = [ $0, $1, $2, ...]; */     \
      size_t j,k;                                                      \
      emitf(state, "%s = [\n", name);                                  \
      indent(state);                                                   \
      for (j = 0; j < nelems; j += stride) {                           \
        for (k = j; k < j + stride; k++)                               \
          emitf(state, (k <  nelems - 1) ? FMT ", " : FMT, *pvalue++); \
        emitf(state, "\n");                                            \
      }                                                                \
      outdent(state);                                                  \
      emitf(state, "];\n");                                            \
    }                                                                  \
  } while (0)

/* ----------------------------------------------------------------------- */

static ztresult_t savehandler_uchar(const char      *name,
                                    const ztuchar_t *pvalue,
                                    size_t           nelems,
                                    size_t           stride,
                                    void            *opaque)
{
  savestate_t *state = opaque;
  DUMP(byte_t);
  return ztresult_OK;
}

static ztresult_t savehandler_ushort(const char       *name,
                                     const ztushort_t *pvalue,
                                     size_t            nelems,
                                     size_t            stride,
                                     void             *opaque)
{
  savestate_t *state = opaque;
  DUMP(half_t);
  return ztresult_OK;
}

static ztresult_t savehandler_uint(const char     *name,
                                   const ztuint_t *pvalue,
                                   size_t          nelems,
                                   size_t          stride,
                                   void           *opaque)
{
  savestate_t *state = opaque;
  DUMP(word_t);
  return ztresult_OK;
}

static ztresult_t savehandler_index(const char *name,
                                    ztindex_t   index,
                                    void       *opaque)
{
  savestate_t *state = opaque;
  if (index == ULONG_MAX)
    emitf(state, "%s = nil;\n", name);
  else
    emitf(state, "%s = %d;\n", name, index);
  return ztresult_OK;
}

static ztresult_t savehandler_version(const char *name,
                                      ztversion_t version,
                                      void       *opaque)
{
  savestate_t *state = opaque;
  emitf(state, "%s = %.2f;\n", name, (double) version / 100.0);
  return ztresult_OK;
}

static ztresult_t savehandler_startstruct(const char *name, void *opaque)
{
  ztresult_t         rc;
  savestate_t       *state   = opaque;
  savestack_entry_t *scope   = NULL;
  int                isarray = 0;
  savestack_entry_t  entry;

  if (!savestack_empty(&state->stack))
  {
    scope   = savestack_top(&state->stack);
    isarray = (scope->container == Array);
  }

  entry.container = Struct;
  entry.nelems    = -1;
  entry.index     = 0;
  rc = savestack_push(&state->stack, &entry);
  if (rc)
    return rc;

  if (isarray)
  {
    scope->index++;
    emitf(state, "{\n");
  }
  else
  {
    emitf(state, "%s = {\n", name);
  }

  indent(state);

  return ztresult_OK;
}

static ztresult_t savehandler_endstruct(void *opaque)
{
  savestate_t       *state   = opaque;
  savestack_entry_t *scope   = NULL;
  int                isarray = 0;
  const char        *end;

  savestack_pop(&state->stack);

  if (!savestack_empty(&state->stack))
  {
    scope   = savestack_top(&state->stack);
    isarray = (scope->container == Array);
  }

  outdent(state);

  if (isarray)
  {
    if (scope->index < scope->nelems)
      end = "},\n";
    else
      end = "}\n";
  }
  else
  {
    end = "};\n";
  }
  emitf(state, end);

  return ztresult_OK;
}

static ztresult_t savehandler_startarray(const char *name,
                                         int         nelems,
                                         void       *opaque)
{
  ztresult_t        rc;
  savestate_t      *state = opaque;
  savestack_entry_t entry;

  entry.container = Array;
  entry.nelems    = nelems;
  entry.index     = 0;
  rc = savestack_push(&state->stack, &entry);
  if (rc)
    return rc;

  emitf(state, "%s = [\n", name);
  indent(state);

  return ztresult_OK;
}

static ztresult_t savehandler_endarray(void *opaque)
{
  savestate_t *state = opaque;

  outdent(state);
  emitf(state, "];\n");

  savestack_pop(&state->stack);

  return ztresult_OK;
}

static ztresult_t savehandler_custom(const char *name,
                                     int         customid,
                                     const void *value,
                                     void       *opaque)
{
  ztresult_t   rc;
  savestate_t *state = opaque;
  char         buf[100];

  if (customid < 0 || customid >= state->nsavers)
    return ztresult_BAD_CUSTOMID;

  buf[0] = '\0';

  rc = state->savers[customid](value, buf, sizeof(buf));
  if (rc)
    return rc;

  emitf(state, "%s = %s;\n", name, buf);

  return ztresult_OK;
}

/* ----------------------------------------------------------------------- */

ztresult_t zt_save(const ztstruct_t  *metastruct,
                   const void        *structure,
                   const char        *filename,
                   const ztregion_t  *regions,
                   int                nregions,
                   ztsaver_t        **savers,
                   int                nsavers)
{
  static const ztwalkhandlers_t savehandlers =
  {
    savehandler_uchar,
    savehandler_ushort,
    savehandler_uint,
    savehandler_index,
    savehandler_version,
    savehandler_startstruct,
    savehandler_endstruct,
    savehandler_startarray,
    savehandler_endarray,
    savehandler_custom
  };

  ztresult_t  rc;
  savestate_t state;

  assert(metastruct);
  assert(structure);
  assert(filename);
  /* regions may be NULL */
  assert(nregions >= 0);
  /* savers may be NULL */
  assert(nsavers >= 0);

  state.f = fopen(filename, "wb");
  if (state.f == NULL)
    return ztresult_BAD_FOPEN;
  
  state.indent_is_due = 0;
  state.depth         = 0;
  savestack_setup(&state.stack);
  state.savers        = savers;
  state.nsavers       = nsavers;

  rc = zt_walk(metastruct, structure, regions, nregions, &savehandlers, &state);
  if (rc)
    goto err;

  savestack_destroy(&state.stack);
  fclose(state.f);

  return ztresult_OK;


err:
  savestack_destroy(&state.stack);
  fclose(state.f);

  return rc;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
