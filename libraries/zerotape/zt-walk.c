/* zt-walk.c */

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zerotape/zerotape.h"

#include "zt-walk.h"

ztresult_t zt_walk(const ztstruct_t       *metastruct,
                   const void             *structure,
                   const ztregion_t       *regions,
                   int                     nregions,
                   const ztwalkhandlers_t *walkhandlers,
                   void                   *opaque)
{
  int              rc;
  const ztfield_t *f;

  for (f = &metastruct->fields[0]; f < &metastruct->fields[metastruct->nfields]; f++)
  {
    const void *rawvalue = (const char *) structure + f->offset;
    int         stride   = (f->stride == ZT_NO_STRIDE) ? f->nelems : f->stride;

    switch (f->type)
    {
    case zttype_uchar:
      {
        const ztuchar_t *pvalue = rawvalue;
        rc = walkhandlers->uchar(f->name, pvalue, f->nelems, stride, opaque);
        if (rc)
          return rc;
        break;
      }

    case zttype_ucharptr:
      {
        const ztuchar_t **ppdata = (const ztuchar_t **) rawvalue;
        const ztuchar_t  *pvalue = *ppdata;
        rc = walkhandlers->uchar(f->name, pvalue, f->nelems, stride, opaque);
        if (rc)
          return rc;
        break;
      }

    case zttype_ushort:
      {
        const ztushort_t *pvalue = rawvalue;
        rc = walkhandlers->ushort(f->name, pvalue, f->nelems, stride, opaque);
        if (rc)
          return rc;
        break;
      }

    case zttype_ushortptr:
      {
        const ztushort_t **ppdata = (const ztushort_t **) rawvalue;
        const ztushort_t  *pvalue = *ppdata;
        rc = walkhandlers->ushort(f->name, pvalue, f->nelems, stride, opaque);
        if (rc)
          return rc;
        break;
      }

    case zttype_uint:
      {
        const ztuint_t *pvalue = rawvalue;
        rc = walkhandlers->uint(f->name, pvalue, f->nelems, stride, opaque);
        if (rc)
          return rc;
        break;
      }

    case zttype_uintptr:
      {
        const ztuint_t **ppdata = (const ztuint_t **) rawvalue;
        const ztuint_t  *pvalue = *ppdata;
        rc = walkhandlers->uint(f->name, pvalue, f->nelems, stride, opaque);
        if (rc)
          return rc;
        break;
      }

    case zttype_struct:
      {
        const void *pstruct = rawvalue;

        if (f->nelems == 1)
        {
          rc = walkhandlers->startstruct(f->name, opaque);
          if (rc)
            return rc;

          rc = zt_walk(f->metadata,
                       pstruct,
                       regions,
                       nregions,
                       walkhandlers,
                       opaque);
          if (rc)
            return rc;

          rc = walkhandlers->endstruct(opaque);
          if (rc)
            return rc;
        }
        else
        {
          int i;

          rc = walkhandlers->startarray(f->name, f->nelems, opaque);
          if (rc)
            return rc;

          for (i = 0; i < f->nelems; i++)
          {
            rc = walkhandlers->startstruct(NULL /* name */, opaque);
            if (rc)
              return rc;

            rc = zt_walk(f->metadata,
                         (char *) pstruct + i * f->size,
                         regions,
                         nregions,
                         walkhandlers,
                         opaque);
            if (rc)
              return rc;

            rc = walkhandlers->endstruct(opaque);
            if (rc)
              return rc;
          }

          rc = walkhandlers->endarray(opaque);
          if (rc)
            return rc;
        }
        break;
      }

    case zttype_structptr:
      {
        const unsigned char **ppstruct = (const unsigned char **) rawvalue;

        if (f->nelems == 1)
        {
          rc = walkhandlers->startstruct(f->name, opaque);
          if (rc)
            return rc;

          rc = zt_walk(f->metadata,
                       *ppstruct,
                       regions,
                       nregions,
                       walkhandlers,
                       opaque);
          if (rc)
            return rc;

          rc = walkhandlers->endstruct(opaque);
          if (rc)
            return rc;
        }
        else
        {
          int j;

          rc = walkhandlers->startarray(f->name, f->nelems, opaque);
          if (rc)
            return rc;

          for (j = 0; j < f->nelems; j++)
          {
            rc = walkhandlers->startstruct(NULL, opaque);
            if (rc)
              return rc;

            rc = zt_walk(f->metadata,
                         *ppstruct++,
                         regions,
                         nregions,
                         walkhandlers,
                         opaque);
            if (rc)
              return rc;

            rc = walkhandlers->endstruct(opaque);
            if (rc)
              return rc;
          }

          rc = walkhandlers->endarray(opaque);
          if (rc)
            return rc;
        }
        break;
      }

    case zttype_staticarrayidx:
      {
        const ztarray_t *array;
        const char      *base;
        const char      *end;
        const char      *pp;
        const char      *ptr;
        unsigned long    index;

        if (f->nelems != 1)
          return ztresult_BAD_FIELD;

        array = f->array;
        base  = array->base;
        end   = (const char *) array->base + array->length;
        pp    = (const char *) structure + f->offset;
        ptr   = *((const char **) pp);

        if (ptr == NULL)
        {
          index = ULONG_MAX;
        }
        else
        {
          if (ptr < base || ptr >= end)
            return ztresult_BAD_POINTER;

          index = (ptr - base) / (array->length / array->nelems);
        }
        rc = walkhandlers->index(f->name, index, opaque);
        if (rc)
          return rc;

        break;
      }

    case zttype_arrayidx:
      {
        int                   r;
        const ztarray_t      *array;
        const char           *base;
        const char           *end;
        const unsigned char **pp;
        const char           *ptr;
        unsigned long         index;

        if (f->nelems != 1)
          return ztresult_BAD_FIELD;

        /* find f->regionid in regions[] */
        for (r = 0; r < nregions; r++)
          if (regions[r].id == f->regionid)
            break;

        if (r == nregions)
          return ztresult_UNKNOWN_REGION;

        /* use the array spec to turn it into an index */
        array = &regions[r].spec;
        base  = array->base;
        end   = (const char *) array->base + array->length;
        pp    = (const unsigned char **) rawvalue;
        ptr   = *((const char **) pp);

        if (ptr == NULL)
        {
          index = ULONG_MAX;
        }
        else
        {
          if (ptr < base || ptr >= end)
            return ztresult_BAD_POINTER;

          index = (ptr - base) / (array->length / array->nelems);
        }
        rc = walkhandlers->index(f->name, index, opaque);
        if (rc)
          return rc;

        break;
      }

    case zttype_version:
      {
        const ztversion_t *pvalue = rawvalue;
        rc = walkhandlers->version(f->name, *pvalue, opaque);
        if (rc)
          return rc;
        break;
      }

    case zttype_custom:
      {
        const void *pvalue = rawvalue;
        rc = walkhandlers->custom(f->name, f->typeidx, pvalue, opaque);
        if (rc)
          return rc;
        break;
      }

    default:
      return ztresult_UNKNOWN_TYPE;
    }
  }

  return ztresult_OK;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
