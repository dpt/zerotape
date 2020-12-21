/* zerotape.h
 *
 * zerotape serialisation library
 *
 * Copyright (c) David Thomas, 2020-2021
 */

#ifndef ZEROTAPE_H
#define ZEROTAPE_H

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

/* ----------------------------------------------------------------------- */

#define ZEROTAPE_VERSION "0.0.2"

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
#define ztresult_BAD_CUSTOMID   ((ztresult_t) 0xA0)

/* ----------------------------------------------------------------------- */

/* AST definitions
 *
 * These are exposed to allow custom type callbacks to interpret expressions.
 */

typedef struct ztast_program ztast_program_t;
typedef struct ztast_statement ztast_statement_t;
typedef struct ztast_assignment ztast_assignment_t;
typedef struct ztast_id ztast_id_t;
typedef struct ztast_expr ztast_expr_t;
typedef struct ztast_value ztast_value_t;
typedef struct ztast_scope ztast_scope_t;
typedef struct ztast_intarray ztast_intarray_t;
typedef struct ztast_intarrayinner ztast_intarrayinner_t;
typedef struct ztast_scopearray ztast_scopearray_t;
typedef struct ztast_scopearrayinner ztast_scopearrayinner_t;

/** A program is a list of statements. */
struct ztast_program
{
  struct ztast_statement *statements;
};

/** A statement can be an assignment. */
struct ztast_statement
{
  struct ztast_statement *next; /* linked list */
  enum ztast_statement_type
  {
    ZTSTMT_ASSIGNMENT
  }
  type;
  union ztast_statement_data
  {
    struct ztast_assignment *assignment;
  }
  u;
};

/** An assignment applies an expression to an ID. */
struct ztast_assignment
{
  struct ztast_id   *id;
  struct ztast_expr *expr;
};

/** An ID holds the name of a field. */
struct ztast_id
{
  char name[1];
};

/** An expression can be a value, an integer array or a scope array. */
struct ztast_expr
{
  enum ztast_expr_type
  {
    ZTEXPR_VALUE,
    ZTEXPR_SCOPE,
    ZTEXPR_INTARRAY,
    ZTEXPR_SCOPEARRAY
  }
  type;
  union ztast_expr_data
  {
    struct ztast_value *value;
    struct ztast_scope *scope;
    struct ztast_intarray *intarray;
    struct ztast_scopearray *scopearray;
  }
  data;
};

/** A value can be an integer, a decimal, or nil. */
struct ztast_value
{
  enum ztast_value_type
  {
    ZTVAL_INTEGER,
    ZTVAL_DECIMAL,
    ZTVAL_NIL
  }
  type;
  union ztast_value_data
  {
    int integer;
    int decimal;
  }
  data;
};

/** A scope is a list of statements. */
struct ztast_scope
{
  struct ztast_statement *statements;
};

/** An intarray points to an intarrayinner, where present. */
struct ztast_intarray
{
  struct ztast_intarrayinner *inner; /* or NULL */
};

/** An intarrayinner is a growable array of integers. */
struct ztast_intarrayinner
{
  int           nused;
  int           nallocated;
  unsigned int *ints;
};

/** A scopearray points to an scopearrayinner, where present. */
struct ztast_scopearray
{
  struct ztast_scopearrayinner *inner; /* or NULL */
};

/** A scopearrayinner is a growable array of pointers to scope. */
struct ztast_scopearrayinner
{
  int                  nused;
  int                  nallocated;
  struct ztast_scope **scopes;
};

/* ----------------------------------------------------------------------- */

/** A metadata field type. */
typedef enum zttype
{
  /** Field is a single byte, or an array of bytes, which will be stored in
   * "field = $xx;", or "field = [ $xx, ... ];", format. */
  zttype_uchar,

  /** Like zttype_uchar but the field is a pointer to a uchar, or array of
   * uchars. */
  zttype_ucharptr,

  /** Field is a single short, or an array of shorts, which will be stored in
   * "field = $xx;", or "field = [ $xx, ... ];", format. */
  zttype_ushort,

  /** Like zttype_ushort but the field is a pointer to a ushort, or array of
   * ushorts. */
  zttype_ushortptr,

  /** Field is a single int, or an array of ints, which will be stored in
   * "field = $xx;", or "field = [ $xx, ... ];", format. */
  zttype_uint,

  /** Like zttype_uint but the field is a pointer to a uint, or array of
   * uints. */
  zttype_uintptr,

  /** Field is an inline structure, or an array of structures, which will be
   * stored in "field = { ... };" format. The structure's layout is defined
   * by the 'metadata' field. */
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
  zttype_version,

  /** Field is a custom type. */
  zttype_custom
}
zttype_t;

/** Identifies a dynamic/heap array. */
typedef const char *ztregionidx_t;

/** Identifies a custom field. */
typedef unsigned int ztcustomid_t;

/* TODO: Work should be done to reduce the size of ztfield_t.
 * - 'type' could fit in just four bits.
 * - 'offset' and 'size' are unlikely to need the full size of size_t.
 * - Since 'metadata' and 'array' can't occur simultaneously we might be able to
 *   fold them into a single pointer. e.g. "const void *metadata_or_array;"
 */

/** Describes a field within an aggregate. */
typedef struct ztfield
{
  /* Fields that concern the member itself. */

  zttype_t               type;     /**< its type */
  const char            *name;     /**< name of member */
  size_t                 offset;   /**< byte offset from start of parent structure */
  size_t                 size;     /**< its size in bytes (char=1) */
  int                    nelems;   /**< how many of it there are */
  ztcustomid_t           typeidx;  /**< custom type id */

  /* Fields that concern the member's pointed-to data. */

  int                    stride;   /**< how wide to layout 2D arrays in saves */
  const struct ztstruct *metadata; /**< definition of an inline struct, or a pointed-to struct */
  const struct ztarray  *array;    /**< description of a pointed-into region */
  ztregionidx_t          regionid; /**< heap array id */
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
  ztregionidx_t id;   /**< an identifier of the heap array */
  ztarray_t     spec; /**< description of the heap array */
}
ztregion_t;

/** A function which interprets the value at pvalue and writes a string
 * representation of it into buf. */
typedef ztresult_t (ztsaver_t)(const void *pvalue, char *buf, size_t bufsz);

/** A function which interprets the AST expression at expr and writes its value
 * to pvalue. */
typedef ztresult_t (ztloader_t)(const ztast_expr_t *expr,
                                void               *pvalue,
                                char              **syntax_error);

/* ----------------------------------------------------------------------- */

typedef unsigned char  ztuchar_t;
typedef unsigned short ztushort_t;
typedef unsigned int   ztuint_t;
typedef unsigned long  ztindex_t;
typedef int            ztversion_t;

/* ----------------------------------------------------------------------- */

#define ZT_NO_STRIDE   (0)
#define ZT_NO_DEFN     (NULL)
#define ZT_NO_ARRAY    (NULL)
#define ZT_NO_REGIONID (NULL)
#define ZT_NO_CUSTOMID (0)

/* CHAR TYPES */

/** Declare a byte field */
#define ZTUCHAR(NAME, STRCT) \
  { zttype_uchar, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) byte field */
#define ZTUCHARPTR(NAME, STRCT) \
  { zttype_ucharptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t *), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare a byte array field */
#define ZTUCHARARRAY(NAME, STRCT, N) \
  { zttype_uchar, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) byte array field */
#define ZTUCHARARRAYPTR(NAME, STRCT, N) \
  { zttype_ucharptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t *), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare a 2D byte array field */
#define ZTUCHARARRAY2D(NAME, STRCT, N, STRIDE) \
  { zttype_uchar, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t), N, ZT_NO_CUSTOMID, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) 2D byte array field */
#define ZTUCHARARRAY2DPTR(NAME, STRCT, N, STRIDE) \
  { zttype_ucharptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuchar_t *), N, ZT_NO_CUSTOMID, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/* SHORT TYPES */

/** Declare a halfword field */
#define ZTUSHORT(NAME, STRCT) \
  { zttype_ushort, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) halfword field */
#define ZTUSHORTPTR(NAME, STRCT) \
  { zttype_ushortptr, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t *), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare a halfword array field */
#define ZTUSHORTARRAY(NAME, STRCT, N) \
  { zttype_ushort, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) halfword array field */
#define ZTUSHORTARRAYPTR(NAME, STRCT, N) \
  { zttype_ushortptr, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t *), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare a 2D halfword array field */
#define ZTUSHORTARRAY2D(NAME, STRCT, N, STRIDE) \
  { zttype_ushort, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t), N, ZT_NO_CUSTOMID, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) 2D halfword array field */
#define ZTUSHORTARRAY2DPTR(NAME, STRCT, N, STRIDE) \
  { zttype_ushortptr, #NAME, offsetof(STRCT, NAME), sizeof(ztushort_t *), N, ZT_NO_CUSTOMID, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/* INT TYPES */

/** Declare a word field */
#define ZTUINT(NAME, STRCT) \
  { zttype_uint, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) word field */
#define ZTUINTPTR(NAME, STRCT) \
  { zttype_uintptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t *), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare a word array field */
#define ZTUINTARRAY(NAME, STRCT, N) \
  { zttype_uint, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) word array field */
#define ZTUINTARRAYPTR(NAME, STRCT, N) \
  { zttype_uintptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t *), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare a 2D word array field */
#define ZTUINTARRAY2D(NAME, STRCT, N, STRIDE) \
  { zttype_uint, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t), N, ZT_NO_CUSTOMID, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) 2D word array field */
#define ZTUINTARRAY2DPTR(NAME, STRCT, N, STRIDE) \
  { zttype_uintptr, #NAME, offsetof(STRCT, NAME), sizeof(ztuint_t *), N, ZT_NO_CUSTOMID, STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/* STRUCT TYPES */

/** Declare an inline structure field */
#define ZTSTRUCT(NAME, STRCT, T, DEFN) \
  { zttype_struct, #NAME, offsetof(STRCT, NAME), sizeof(T), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) structure field */
#define ZTSTRUCTPTR(NAME, STRCT, T, DEFN) \
  { zttype_structptr, #NAME, offsetof(STRCT, NAME), sizeof(T), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an inline structure array field */
#define ZTSTRUCTARRAY(NAME, STRCT, T, N, DEFN) \
  { zttype_struct, #NAME, offsetof(STRCT, NAME), sizeof(T), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/** Declare an indirect (pointed-to) structure array field */
#define ZTSTRUCTPTRARRAY(NAME, STRCT, T, N, DEFN) \
  { zttype_structptr, #NAME, offsetof(STRCT, NAME), sizeof(T), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/* ARRAY INDEX TYPES */

/** Declare a pointer which indexes a compile-time allocated array */
#define ZTARRAYIDX_STATIC(NAME, STRCT, T, ARRAY) \
  { zttype_staticarrayidx, #NAME, offsetof(STRCT, NAME), sizeof(T), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ARRAY, ZT_NO_REGIONID }

/** Declare an array of pointers which index the same compile-time allocated array */
#define ZTARRAYIDXARRAY_STATIC(NAME, STRCT, T, N, ARRAY) \
  { zttype_staticarrayidx, #NAME, offsetof(STRCT, NAME), sizeof(T), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ARRAY, ZT_NO_REGIONID }

/** Declare a pointer which indexes a run-time allocated array */
#define ZTARRAYIDX(NAME, STRCT, T, HEAPID) \
  { zttype_arrayidx, #NAME, offsetof(STRCT, NAME), sizeof(T), 1, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, HEAPID }

/** Declare an array of pointers which index the same run-time allocated array */
#define ZTARRAYIDXARRAY(NAME, STRCT, T, N, HEAPID) \
  { zttype_arrayidx, #NAME, offsetof(STRCT, NAME), sizeof(T), N, ZT_NO_CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, HEAPID }

/* CUSTOM TYPES */

#define ZTCUSTOM(NAME, STRCT, CUSTOMID) \
  { zttype_custom, #NAME, offsetof(STRCT, NAME), sizeof(void *), 1, CUSTOMID , ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID}

#define ZTCUSTOMARRAY(NAME, STRCT, N, CUSTOMID) \
  { zttype_custom, #NAME, offsetof(STRCT, NAME), sizeof(void *), N, CUSTOMID, ZT_NO_STRIDE, ZT_NO_DEFN, ZT_NO_ARRAY, ZT_NO_REGIONID }

/* ----------------------------------------------------------------------- */

/**
 * Load
 *
 * \param meta description of 'structure'
 * \param structure structure to load
 * \param filename filename to load from
 * \param regions runtime heap array specs
 * \param nregions number of heap array specs
 * \param loaders array of loader functions - one per custom ID
 * \param nloaders number of loader functions
 * \param syntax_error syntax error message(s) - dispose using zt_freesyntax()
 */
ztresult_t zt_load(const ztstruct_t  *meta,
                   void              *structure,
                   const char        *filename,
                   const ztregion_t  *regions,
                   int                nregions,
                   ztloader_t       **loaders,
                   int                nloaders,
                   char             **syntax_error);

/**
 * Dispose of a syntax error.
 *
 * \param syntax_error syntax error to dispose of
 */
void zt_freesyntax(char *syntax_error);

/* ----------------------------------------------------------------------- */

/**
 * Save
 *
 * \param meta description of 'structure'
 * \param structure structure to save
 * \param filename filename to save to
 * \param regions runtime heap array specs
 * \param nregions number of heap array specs
 * \param savers array of saver functions - one per custom ID
 * \param nsavers number of saver functions
 */
ztresult_t zt_save(const ztstruct_t *meta,
                   const void       *structure,
                   const char       *filename,
                   const ztregion_t *regions,
                   int               nregions,
                   ztsaver_t       **savers,
                   int               nsavers);

/* ----------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* ZEROTAPE_H */

/* vim: set ts=8 sts=2 sw=2 et: */
