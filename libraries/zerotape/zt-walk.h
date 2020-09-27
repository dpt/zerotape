/* zt-walk.h */

#ifndef ZT_WALK_H
#define ZT_WALK_H

#include <stddef.h>

#include "zerotape/zerotape.h"

typedef struct zt_walkhandlers
{
  ztresult_t (*uchar)(const char *name, const ztuchar_t *values, size_t nelems, size_t stride, void *opaque);
  ztresult_t (*ushort)(const char *name, const ztushort_t *values, size_t nelems, size_t stride, void *opaque);
  ztresult_t (*uint)(const char *name, const ztuint_t *values, size_t nelems, size_t stride, void *opaque);
  ztresult_t (*index)(const char *name, ztindex_t index, void *opaque);
  ztresult_t (*version)(const char *name, ztversion_t value, void *opaque);
  ztresult_t (*startstruct)(const char *name, void *opaque);
  ztresult_t (*endstruct)(void *opaque);
  ztresult_t (*startarray)(const char *name, int nelems, void *opaque);
  ztresult_t (*endarray)(void *opaque);
  ztresult_t (*custom)(const char *name, int customid, const void *value, void *opaque);
}
ztwalkhandlers_t;

ztresult_t zt_walk(const ztstruct_t       *metastruct,
                   const void             *structure,
                   const ztregion_t       *regions,
                   int                     nregions,
                   const ztwalkhandlers_t *walkhandlers,
                   void                   *opaque);

#endif /* ZT_WALK_H */

/* vim: set ts=8 sts=2 sw=2 et: */
