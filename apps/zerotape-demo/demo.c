/* demo/demo.c
 *
 * An example of using zerotape
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zerotape/zerotape.h"

/* ----------------------------------------------------------------------- */

/*
 * THE USER'S STUFF
 */

/* A sub-structure used in a couple of the examples. */
typedef struct sub
{
  unsigned char value;
}
sub_t;

/* Example structure. */
typedef struct example
{
  unsigned int  integer;
  unsigned int *pointer_to_integer;

  unsigned int  integer_array[3];

  sub_t         inline_sub;
  sub_t        *pointer_to_sub;
  sub_t         array_of_sub[3];

  const char   *static_pointer; /* indexes example_array[] which is static */
  const char   *static_nullpointer; /* same, but set to NULL */
  const char   *pointer; /* indexes a malloced block */
  const char   *nullpointer; /* same, but set to NULL */

  /* This custom field is not representable with stock zerotape types. It
   * points at one of popular_beat_combo[], but we want to store it as an
   * index into that array, so it needs custom load and save functions. */
  const char   *string_in_array;
}
example_t;

/* A table of strings used by the custom field example. */
static const char *popular_beat_combo[] =
{
  "John",
  "Paul",
  "George",
  "Ringo"
};

/* A value that gets pointed at. */
static unsigned int pointed_at = 33;

/* An array that static_pointer indexes. */
static const char example_array[] = { 44, 55, 66 };

/* ----------------------------------------------------------------------- */

/*
 * ZEROTAPE'S STUFF
 */

/* Describes 'example_array'. */
static const ztarray_t example_array_desc =
{
  &example_array[0],
  sizeof(example_array),
  NELEMS(example_array)
};

/* Identifies an array unknown at compile time. */
static const char tenbyte_id[] = "some array";

/* Describes the 'sub_t' fields. */
static const ztfield_t substruct_fields[] =
{
  ZTUCHAR(value, sub_t)
};

/* Describes a 'sub_t' itself. */
static const ztstruct_t substruct_meta =
{
  NELEMS(substruct_fields),
  substruct_fields
};

/* An ID for our custom type. */
#define CUSTOMTYPE_BAND_MEMBER 0

/* This is the zerotape description of your struct's fields. Note that it
 * doesn't have to define every part of your struct: just the parts you want to
 * preserve. (It currently doesn't necessarily have to be in the same order as
 * the original struct either, but maybe don't rely on that.) */
static const ztfield_t example_fields[] =
{
  ZTUINT(integer, example_t),
  ZTUINTPTR(pointer_to_integer, example_t),
  ZTUINTARRAY(integer_array, example_t, 3),
  ZTSTRUCT(inline_sub, example_t, sub_t, &substruct_meta),
  ZTSTRUCTPTR(pointer_to_sub, example_t, sub_t *, &substruct_meta),
  ZTSTRUCTARRAY(array_of_sub, example_t, sub_t, 3, &substruct_meta),
  /* 'static_pointer' points at somewhere in example_array, so a description
   * for that is created in example_array_desc and referenced. An integer
   * array index will be output. */
  ZTARRAYIDX_STATIC(static_pointer, example_t, const char *, &example_array_desc),
  ZTARRAYIDX_STATIC(static_nullpointer, example_t, const char *, &example_array_desc),
  /* 'pointer' points at somewhere in a block that's unknown at compile time, so
   * we assign it an array ID which will be used. An integer array index will be
   * output. */
  ZTARRAYIDX(pointer, example_t, const char *, tenbyte_id),
  ZTARRAYIDX(nullpointer, example_t, const char *, tenbyte_id),
  ZTCUSTOM(string_in_array, example_t, CUSTOMTYPE_BAND_MEMBER)
};

/* This is the zerotape description of your struct. */
static const ztstruct_t example_meta =
{
  NELEMS(example_fields),
  example_fields
};

/* ----------------------------------------------------------------------- */

/*
 * CUSTOM FIELD HANDLERS
 */

/* This is a custom value save handler. It takes the value from the struct
 * and formats it into text so it can be output to file. */
ztresult_t bandmember_saver(const void *pvoidval, char *buf, size_t bufsz)
{
  const char **ppcharval = (const char **) pvoidval;
  int          i;

  for (i = 0; i < NELEMS(popular_beat_combo); i++)
    if (popular_beat_combo[i] == *ppcharval)
      break;
  if (i == NELEMS(popular_beat_combo))
    return ztresult_BAD_POINTER;

  sprintf(buf, "%d", i);
  return ztresult_OK;
}

/* This is a custom value load handler. It receives the input AST and
 * interprets it, producing a value to be stored to the struct.
 *
 * The AST can only ever be composed of elements of which zerotape's lexer
 * and parser are aware. So, for example, if you wanted a quoted string to be
 * available here, it would be more work elsewhere.
 *
 * Since you will receive whatever AST has been given in the input file you
 * must be careful to validate as much as possible. The AST could contain
 * *any* valid program expression.
 */
ztresult_t bandmember_loader(const ztast_expr_t *expr,
                             void               *pvoidval,
                             char               *errbuf)
{
  const char **ppcharval = (const char **) pvoidval;
  int          index;

  if (expr->type != ZTEXPR_VALUE)
  {
    strcpy(errbuf, "value type required (custom)"); /* ztsyntx_NEED_VALUE */
    return ztresult_SYNTAX_ERROR;
  }
  if (expr->data.value->type != ZTVAL_INTEGER)
  {
    strcpy(errbuf, "integer type required (custom)"); /* ztsyntx_NEED_INTEGER */
    return ztresult_SYNTAX_ERROR;
  }
  index = expr->data.value->data.integer;
  if (index < 0 || index >= NELEMS(popular_beat_combo))
  {
    strcpy(errbuf, "value out of range (custom)"); /* ztsyntx_VALUE_RANGE */
    return ztresult_SYNTAX_ERROR;
  }

  *ppcharval = popular_beat_combo[index];
  *errbuf = '\0';
  return ztresult_OK;
} 

/* ----------------------------------------------------------------------- */

int main(int argc, const char *argv[])
{
#ifdef __riscos
  static const char testfile[] = "demo_zt";
#else
  static const char testfile[] = "demo.zt";
#endif

  ztresult_t  rc;
  char       *tenbyte;
  example_t   example;
  ztregion_t  regions[1]; /* descriptions of heap blocks */
  sub_t       sub;
  ztsaver_t  *savers[1];
  ztloader_t *loaders[1];
  char       *syntax_error;

  tenbyte = malloc(10);
  if (tenbyte == NULL)
    return EXIT_FAILURE;

  sub.value = 51;

  example.integer               = 42;
  example.pointer_to_integer    = &pointed_at;
  example.integer_array[0]      = 61;
  example.integer_array[1]      = 62;
  example.integer_array[2]      = 63;
  example.inline_sub.value      = 43;
  example.pointer_to_sub        = &sub;
  example.array_of_sub[0].value = 44;
  example.array_of_sub[1].value = 45;
  example.array_of_sub[2].value = 46;
  example.static_pointer        = &example_array[2];
  example.static_nullpointer     = NULL;
  example.pointer               = &tenbyte[5];
  example.nullpointer            = NULL;
  example.string_in_array       = popular_beat_combo[3];

  regions[0].id                 = tenbyte_id;
  regions[0].spec.base          = tenbyte;
  regions[0].spec.length        = 10;
  regions[0].spec.nelems        = 10;

  savers[CUSTOMTYPE_BAND_MEMBER] = bandmember_saver;
  rc = zt_save(&example_meta,
               &example,
                testfile,
               &regions[0],
                NELEMS(regions),
                savers,
                NELEMS(savers));
  if (rc != ztresult_OK)
  {
    fprintf(stderr, "zt_save failed (%d)\n", rc);
    return EXIT_FAILURE;
  }

  /* Setup the example structure layout (but not the values themselves) */
  memset(&example, 0x55, sizeof(example_t));
  example.pointer_to_integer    = &pointed_at;
  example.pointer_to_sub        = &sub;

  loaders[CUSTOMTYPE_BAND_MEMBER] = bandmember_loader;
  rc = zt_load(&example_meta,
               &example,
                testfile,
               &regions[0],
                NELEMS(regions),
                loaders,
               NELEMS(loaders),
               &syntax_error);
  if (rc != ztresult_OK)
  {
    fprintf(stderr, "zt_load failed (%d)\n", rc);
    if (syntax_error)
    {
      fprintf(stderr, "syntax error: %s\n", syntax_error);
      zt_freesyntax(syntax_error);
    }
    return EXIT_FAILURE;
  }

  assert(example.integer == 42);
  assert(pointed_at == 33);
  assert(example.integer_array[0] == 61);
  assert(example.integer_array[1] == 62);
  assert(example.integer_array[2] == 63);
  assert(example.inline_sub.value == 43);
  assert(example.pointer_to_sub->value == 51);
  assert(example.array_of_sub[0].value == 44);
  assert(example.array_of_sub[1].value == 45);
  assert(example.array_of_sub[2].value == 46);
  assert(example.static_pointer == &example_array[2]);
  assert(example.static_nullpointer == NULL);
  assert(example.pointer == &tenbyte[5]);
  assert(example.nullpointer == NULL);
  assert(example.string_in_array == popular_beat_combo[3]);

  return EXIT_SUCCESS;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
