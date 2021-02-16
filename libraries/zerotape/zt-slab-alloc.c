/* zt-slab-alloc.c
 *
 * malloc() replacement which coalesces small blocks into slabs.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zt-slab-alloc.h"

/* ----------------------------------------------------------------------- */

#define SLAB_SIZE  (4000)

#if defined(__LP64__) || defined(_WIN64)
#define SLAB_ALIGN    (8)
#else
#define SLAB_ALIGN    (4)
#endif

/* ----------------------------------------------------------------------- */

struct ztslaballoc
{
  void   **slablist;
  size_t   nslablistused;
  size_t   nslablistalloced;

  size_t   remaining; /* in current slab */

  size_t   total_allocs;
  size_t   current_allocs;
  size_t   total_allocated;
};

/* ----------------------------------------------------------------------- */

ztslaballoc_t *ztslaballoc_create(void)
{
  ztslaballoc_t *sa;

  sa = calloc(1, sizeof(ztslaballoc_t));
  if (sa == NULL)
    return NULL;

  return sa;
}

void ztslaballoc_destroy(ztslaballoc_t *sa)
{
  size_t i;

  if (sa == NULL)
    return;

  /* FIXME: Free any non-slab allocations */

  /* free all slabs */
  for (i = 0; i < sa->nslablistused; i++)
    free(sa->slablist[i]);

  free(sa->slablist);

  free(sa);
}

/* ----------------------------------------------------------------------- */

void *ztslaballoc(size_t n, void *opaque)
{
  ztslaballoc_t *sa = opaque;
  char          *p;

  /* round block sizes up to multiple of SLAB_ALIGN */
  n = (n + SLAB_ALIGN - 1) & ~(SLAB_ALIGN - 1);

  if (n >= SLAB_SIZE)
    return malloc(n);

  if (sa->remaining < n)
  {
    void *newslab;

    if (sa->nslablistused == sa->nslablistalloced)
    {
      size_t alloced;
      void **newlist;

      alloced = sa->nslablistalloced * 2; /* doubling strategy */
      if (alloced <= 0)
        alloced = 4;
      newlist = realloc(sa->slablist, alloced * sizeof(*newlist));
      if (newlist == NULL)
        return NULL;

      sa->slablist         = newlist;
      sa->nslablistalloced = alloced;
    }

    newslab = malloc(SLAB_SIZE);
    if (newslab == NULL)
      return NULL;

    sa->slablist[sa->nslablistused++] = newslab;

    sa->remaining = SLAB_SIZE;
  }

  p = sa->slablist[sa->nslablistused - 1];
  p += SLAB_SIZE - sa->remaining;
  sa->remaining -= n;

  sa->total_allocs++;
  sa->current_allocs++;
  sa->total_allocated += n;

  return p;
}

void ztslabfree(void *p, void *opaque)
{
  ztslaballoc_t *sa = opaque;

  if (p == NULL)
    return;

  sa->current_allocs--;
  /* we don't free in this allocator */
}

/* ----------------------------------------------------------------------- */

#ifdef ZT_DEBUG
void ztslaballoc_spew(ztslaballoc_t *sa)
{
  if (sa == NULL)
    return;

  printf("ztslaballoc:\n");
  printf("- total blocks allocated=%lu\n", sa->total_allocs);
  printf("- current blocks allocated=%lu\n", sa->current_allocs);
  printf("- total bytes allocated=%lu bytes\n", sa->total_allocated);
  printf("- number of slabs=%lu used (of %lu) @ %d each = %lu total bytes\n", sa->nslablistused, sa->nslablistalloced, SLAB_SIZE, sa->nslablistused * SLAB_SIZE);
  printf("- average allocation size=%.2f\n", (double) sa->total_allocated / sa->total_allocs);
}
#endif

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
