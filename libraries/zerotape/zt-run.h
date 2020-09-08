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
 * \param structure structure to populate
 * \param syntax_error error message, or NULL if none
 */
ztresult_t zt_run_program(const ztast_t    *ast,
                          const ztstruct_t *meta,
                          const ztregion_t *regions,
                          int               nregions,
                          void             *structure,
                          char            **syntax_error);

#endif /* ZT_RUN_H */

/* vim: ts=8 sts=2 sw=2 et */
