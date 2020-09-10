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

/* YOUR STUFF */

typedef struct substruct
{
  unsigned char a;
}
substruct_t;

/* This is a struct that you want to serialise. */
typedef struct example
{
  unsigned int  integer;
  unsigned int *pinteger;
  substruct_t   sub;
  substruct_t  *subptr;
  substruct_t   subarray[3];
  const char   *static_pointer; /* indexes example_array[] which is static */
  const char   *pointer; /* indexes a malloced block */
}
example_t;

static unsigned int j_random_int = 77;

/* This is an array that static_pointer indexes. */
static const char example_array[] = { 44, 55, 66 };

/* ----------------------------------------------------------------------- */

/* ZEROTAPE STUFF */

/* This is the zerotape description of 'example_array'. */
static const ztarray_t example_array_desc =
{
  &example_array[0],
  sizeof(example_array),
  NELEMS(example_array)
};

/* This is the zerotape identifier for some array yet-to-be created. */
static const char array_id[] = "some identifier";

static const ztfield_t substruct_fields[] =
{
  ZTUCHAR(a, substruct_t)
};
static const ztstruct_t substruct_meta =
{
  NELEMS(substruct_fields),
  substruct_fields
};

/* This is the zerotape description of your struct's fields. Note that it
 * doesn't have to define every part of your struct; just the parts you want to
 * preserve. (It currently doesn't necessarily have to be in the same order as
 * the original struct either, but don't rely on that.) */
static const ztfield_t example_fields[] =
{
  ZTUINT(integer, example_t),
  ZTUINTPTR(pinteger, example_t),
  ZTSTRUCT(sub, example_t, substruct_t, &substruct_meta),
  ZTSTRUCTPTR(subptr, example_t, substruct_t *, &substruct_meta),
  ZTSTRUCTARRAY(subarray, example_t, substruct_t, 3, &substruct_meta),
  /* 'static_pointer' points at somewhere in example_array, so a description is created in example_array_desc and referenced. An integer array index will be output. */
  ZTARRAYIDX_STATIC(static_pointer, example_t, const char *, &example_array_desc),
  /* 'pointer' points at somewhere in a block that's unknown at compile time, so we assign it an array ID which will be used. An integer array index will be output. */
  ZTARRAYIDX(pointer, example_t, const char *, array_id)
};

/* This is the zerotape description of your struct. */
static const ztstruct_t example_meta =
{
  NELEMS(example_fields),
  example_fields
};

/* ----------------------------------------------------------------------- */

int main(int argc, const char *argv[])
{
  ztresult_t  rc;
  char       *array;
  example_t   example;
  ztregion_t  regions[1]; /* heap blocks */
  substruct_t localsub;

  array = malloc(10);
  if (array == NULL)
    return EXIT_FAILURE;

  localsub.a = 51;

  example.integer        = 42;
  example.pinteger       = &j_random_int;
  example.sub.a          = 43;
  example.subptr         = &localsub;
  example.subarray[0].a  = 44;
  example.subarray[1].a  = 45;
  example.subarray[2].a  = 46;
  example.static_pointer = &example_array[2];
  example.pointer        = &array[5];

  regions[0].id          = array_id;
  regions[0].spec.base   = array;
  regions[0].spec.length = 10;
  regions[0].spec.nelems = 10;

  rc = zt_save(&example_meta, &example, "test.zt", &regions[0], NELEMS(regions));
  if (rc != ztresult_OK)
    return EXIT_FAILURE;

  rc = zt_load(&example_meta, &example, "test.zt", &regions[0], NELEMS(regions));
  if (rc != ztresult_OK)
    return EXIT_FAILURE;

  assert(example.integer == 42);

  return EXIT_SUCCESS;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
