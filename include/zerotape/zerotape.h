/* zerotape.h */

#ifndef ZEROTAPE_H
#define ZEROTAPE_H

#include <stddef.h>

/* ----------------------------------------------------------------------- */

#ifndef NELEMS
#define NELEMS(a) (sizeof(a) / sizeof(a[0]))
#endif

/* ----------------------------------------------------------------------- */

typedef int ztresult_t;

#define ztresult_OK             ((ztresult_t) 0x00)
#define ztresult_OOM            ((ztresult_t) 0x10)
#define ztresult_UNKNOWN_REGION ((ztresult_t) 0x20)
#define ztresult_SYNTAX_ERROR   ((ztresult_t) 0x30)
#define ztresult_NO_PROGRAM     ((ztresult_t) 0x40)
#define ztresult_BAD_FOPEN      ((ztresult_t) 0x50)
#define ztresult_UNKNOWN_TYPE   ((ztresult_t) 0x60)
#define ztresult_PARSE_FAIL     ((ztresult_t) 0x70)
#define ztresult_BAD_POINTER    ((ztresult_t) 0x80)
#define ztresult_BAD_FIELD      ((ztresult_t) 0x90)

/* ----------------------------------------------------------------------- */

/** A metadata field type. */
typedef enum zttype
{
  /** Field is a single byte, or an array of bytes, which will be stored in
   * "field = $xx;", or "field = [ $xx, ... ];", format. */
  zttype_uchar,

  /** Like zttype_uchar but the field is a pointer to a uchar, or array of uchars. */
  zttype_ucharptr,

  /** Field is a single short, or an array of shorts, which will be stored in
   * "field = $xx;", or "field = [ $xx, ... ];", format. */
  zttype_ushort,

  /** Like zttype_ushort but the field is a pointer to a ushort, or array of ushorts. */
  zttype_ushortptr,

  /** Field is a single int, or an array of ints, which will be stored in
   * "field = $xx;", or "field = [ $xx, ... ];", format. */
  zttype_uint,

  /** Like zttype_uint but the field is a pointer to a uint, or array of uints. */
  zttype_uintptr,

  /** Field is an inline structure, or an array of structures, which will be
   * stored in "field = { ... };" format. The structure's layout is defined
   * in 'metadata'. */
  zttype_struct,

  /** Like zttype_struct but the field is a pointer to a struct. */
  zttype_structptr,

  /** Field is a pointer into a compile-time known array, which will be
   * stored in "field = <array index>;" format. The array's layout is defined
   * in 'array'. */
  zttype_staticarrayidx,

  /** Field is a pointer into a heap-allocated array, which will be stored in
   * "field = <array index>;" format. The array is identified by 'regionid'.
   * */
  zttype_arrayidx,

  /** Field is an int in the range 0..999 which will be stored in "field =
   * x.yz;" format. */
  zttype_version
}
zttype_t;

/** Identifies a dynamic/heap array. */
typedef const char *ztregionid_t;

/* TODO: Work should be done to reduce the size of ztfield_t. Since 'metadata'
 * and 'array' can't occur simultaneously we might be able to fold them into a
 * single pointer. e.g. "const void *metadata_or_array;" */

/** Describes a field within an aggregate. */
typedef struct ztfield
{
  zttype_t               type;     /**< its type */
  const char            *name;     /**< name of member */
  int                    offset;   /**< byte offset from start of parent structure */
  size_t                 size;     /**< its size in bytes (char=1) */
  int                    nelems;   /**< how many of it there are */
  int                    stride;   /**< how wide to layout 2D arrays in saves */
  const struct ztstruct *metadata; /**< definition of an inline struct, or a pointed-to struct */
  const struct ztarray  *array;    /**< description of a pointed-into region */
  ztregionid_t           regionid; /**< heap array id */
}
ztfield_t;

/** Defines a representation of an aggregate. */
typedef struct ztstruct
{
  int              nfields; /**< number of fields given in 'fields' */
  const ztfield_t *fields;  /**< array of ztfields */
}
ztstruct_t;

/** Describes a pointed-to array (static: located in the binary). */
typedef struct ztarray
{
  const void *base;   /**< pointer to array */
  size_t      length; /**< total length of array */
  int         nelems; /**< number of elements */
}
ztarray_t;

/** Describes a pointed-to array (dynamic: located in the heap). */
typedef struct ztregion
{
  ztregionid_t id;   /**< an identifier of the heap array */
  ztarray_t    spec; /**< description of the heap array */
}
ztregion_t;

/* ----------------------------------------------------------------------- */

typedef unsigned char  ztuchar_t;
typedef unsigned short ztushort_t;
typedef unsigned int   ztuint_t;
typedef unsigned long  ztindex_t;
typedef int            ztversion_t;

/* ----------------------------------------------------------------------- */

#define ZT_NO_STRIDE (0)
#define ZT_NO_DEFN   (NULL)
#define ZT_NO_ARRAY  (NULL)
#define ZT_NO_HEAPID (NULL)

/* CHAR TYPES */

/** Declare a byte field */
#define ZTUCHAR(NAME, STRCT) \
  { zttype_uchar, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t), 1, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) byte field */
#define ZTUCHARPTR(NAME, STRCT) \
  { zttype_ucharptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t *), 1, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare a byte array field */
#define ZTUCHARARRAY(NAME, STRCT, N) \
  { zttype_uchar, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t), N, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) byte array field */
#define ZTUCHARARRAYPTR(NAME, STRCT, N) \
  { zttype_ucharptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t *), N, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare a 2D byte array field */
#define ZTUCHARARRAY2D(NAME, STRCT, N, STRIDE) \
  { zttype_uchar, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t), N, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) 2D byte array field */
#define ZTUCHARARRAY2DPTR(NAME, STRCT, N, STRIDE) \
  { zttype_ucharptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t *), N, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/* SHORT TYPES */

/** Declare a halfword field */
#define ZTUSHORT(NAME, STRCT) \
  { zttype_ushort, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t), 1, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) halfword field */
#define ZTUSHORTPTR(NAME, STRCT) \
  { zttype_ushortptr, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t *), 1, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare a halfword array field */
#define ZTUSHORTARRAY(NAME, STRCT, N) \
  { zttype_ushort, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t), N, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) halfword array field */
#define ZTUSHORTARRAYPTR(NAME, STRCT, N) \
  { zttype_ushortptr, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t *), N, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare a 2D halfword array field */
#define ZTUSHORTARRAY2D(NAME, STRCT, N, STRIDE) \
  { zttype_ushort, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t), N, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) 2D halfword array field */
#define ZTUSHORTARRAY2DPTR(NAME, STRCT, N, STRIDE) \
  { zttype_ushortptr, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t *), N, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/* INT TYPES */

/** Declare a word field */
#define ZTUINT(NAME, STRCT) \
  { zttype_uint, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t), 1, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) word field */
#define ZTUINTPTR(NAME, STRCT) \
  { zttype_uintptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t *), 1, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare a word array field */
#define ZTUINTARRAY(NAME, STRCT, N) \
  { zttype_uint, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t), N, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) word array field */
#define ZTUINTARRAYPTR(NAME, STRCT, N) \
  { zttype_uintptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t *), N, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare a 2D word array field */
#define ZTUINTARRAY2D(NAME, STRCT, N, STRIDE) \
  { zttype_uint, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t), N, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) 2D word array field */
#define ZTUINTARRAY2DPTR(NAME, STRCT, N, STRIDE) \
  { zttype_uintptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t *), N, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/* STRUCT TYPES */

/** Declare an inline structure field */
#define ZTSTRUCT(NAME, STRCT, T, DEFN) \
  { zttype_struct, #NAME, offsetof(STRCT, NAME), sizeof(T), 1, ZT_NO_STRIDE, DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) structure field */
#define ZTSTRUCTPTR(NAME, STRCT, T, DEFN) \
  { zttype_structptr, #NAME, offsetof(STRCT, NAME), sizeof(T), 1, ZT_NO_STRIDE, DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an inline structure array field */
#define ZTSTRUCTARRAY(NAME, STRCT, T, N, DEFN) \
  { zttype_struct, #NAME, offsetof(STRCT, NAME), sizeof(T), N, ZT_NO_STRIDE, DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/** Declare an indirect (pointed-to) structure array field */
#define ZTSTRUCTPTRARRAY(NAME, STRCT, T, N, DEFN) \
  { zttype_structptr, #NAME, offsetof(STRCT, NAME), sizeof(T), N, ZT_NO_STRIDE, DEFN, ZT_NO_ARRAY, ZT_NO_HEAPID }

/* ARRAY INDEX TYPES */

/** Declare a pointer which indexes a compile-time allocated array */
#define ZTARRAYIDX_STATIC(NAME, STRCT, T, ARRAY) \
  { zttype_staticarrayidx, #NAME, offsetof(STRCT, NAME), sizeof(T), 1, ZT_NO_STRIDE, ZT_NO_DEFN, ARRAY, ZT_NO_HEAPID }

/** Declare an array of pointers which index the same compile-time allocated array */
#define ZTARRAYIDXARRAY_STATIC(NAME, STRCT, T, N, ARRAY) \
  { zttype_staticarrayidx, #NAME, offsetof(STRCT, NAME), sizeof(T), N, ZT_NO_STRIDE, ZT_NO_DEFN, ARRAY, ZT_NO_HEAPID }

/** Declare a pointer which indexes a run-time allocated array */
#define ZTARRAYIDX(NAME, STRCT, T, HEAPID) \
  { zttype_arrayidx, #NAME, offsetof(STRCT, NAME), sizeof(T), 1, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, HEAPID }

/** Declare an array of pointers which index the same run-time allocated array */
#define ZTARRAYIDXARRAY(NAME, STRCT, T, N, HEAPID) \
  { zttype_arrayidx, #NAME, offsetof(STRCT, NAME), sizeof(T), N, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, HEAPID }

/* ----------------------------------------------------------------------- */

/**
 * Load
 *
 * \param meta description of 'structure'
 * \param structure structure to load
 * \param filename filename to load from
 * \param regions runtime heap array specs
 * \param nregions number of heap array specs
 */
ztresult_t zt_load(const ztstruct_t *meta,
                   void             *structure,
                   const char       *filename,
                   const ztregion_t *regions,
                   int               nregions);

/* ----------------------------------------------------------------------- */

/**
 * Save
 *
 * \param meta description of 'structure'
 * \param structure structure to save
 * \param filename filename to save to
 * \param regions runtime heap array specs
 * \param nregions number of heap array specs
 */
ztresult_t zt_save(const ztstruct_t *meta,
                   const void       *structure,
                   const char       *filename,
                   const ztregion_t *regions,
                   int               nregions);

/* ----------------------------------------------------------------------- */

#endif /* ZEROTAPE_H */

/* vim: ts=8 sts=2 sw=2 et */
