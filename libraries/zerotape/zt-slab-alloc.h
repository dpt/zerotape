/* zt-slab-alloc.h
 *
 * malloc() replacement which coalesces small blocks into slabs.
 */

#ifndef ZT_SLAB_ALLOC_H
#define ZT_SLAB_ALLOC_H

/* ----------------------------------------------------------------------- */

typedef struct ztslaballoc ztslaballoc_t;

/* ----------------------------------------------------------------------- */

ztslaballoc_t *ztslaballoc_create(void);
void ztslaballoc_destroy(ztslaballoc_t *sa);

void *ztslaballoc(size_t n, void *opaque);
void ztslabfree(void *p, void *opaque);

void ztslaballoc_spew(ztslaballoc_t *sa);

/* ----------------------------------------------------------------------- */

#endif /* ZT_SLAB_ALLOC_H */

/* vim: set ts=8 sts=2 sw=2 et: */
