/*
  libcexcept - something with cexcept

  Copyright (C) 2011 Someone <someone@example.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _LIBCEXCEPT_H_
#define _LIBCEXCEPT_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * cexcept_ctx
 *
 * library user context - reads the config and system
 * environment, user variables, allows custom logging
 */
struct cexcept_ctx;
struct cexcept_ctx *cexcept_ref(struct cexcept_ctx *ctx);
struct cexcept_ctx *cexcept_unref(struct cexcept_ctx *ctx);
int cexcept_new(struct cexcept_ctx **ctx);
void cexcept_set_log_fn(struct cexcept_ctx *ctx,
                  void (*log_fn)(struct cexcept_ctx *ctx,
                                 int priority, const char *file, int line, const char *fn,
                                 const char *format, va_list args));

/*
 * cexcept_thing
 *
 * access to things of cexcept
 */
struct cexcept_thing;
struct cexcept_thing *cexcept_thing_ref(struct cexcept_thing *thing);
struct cexcept_thing *cexcept_thing_unref(struct cexcept_thing *thing);
struct cexcept_ctx *cexcept_thing_get_ctx(struct cexcept_thing *thing);
int cexcept_thing_new_from_string(struct cexcept_ctx *ctx, const char *string, struct cexcept_thing **thing);
struct cexcept_list_entry *cexcept_thing_get_some_list_entry(struct cexcept_thing *thing);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
