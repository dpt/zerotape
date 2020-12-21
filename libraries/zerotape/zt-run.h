/* zt-run.h */

#ifndef ZT_RUN_H
#define ZT_RUN_H

#include "zerotape/zerotape.h"

#include "zt-ast.h"

/**
 * Execute the given program.
 *
 * \param ast AST
 * \param meta description of 'structure'
 * \param regions runtime heap array specs
 * \param nregions number of heap array specs
 * \param loaders array of loader functions - one per custom ID
 * \param nloaders number of loader functions
 * \param structure structure to populate
 * \param errbuf buffer for error message(s)
 */
ztresult_t zt_run_program(const ztast_t    *ast,
                          const ztstruct_t *meta,
                          const ztregion_t *regions,
                          int               nregions,
                          ztloader_t      **loaders,
                          int               nloaders,
                          void             *structure,
                          char             *errbuf);

#endif /* ZT_RUN_H */

/* vim: set ts=8 sts=2 sw=2 et: */
