/* GNU cexcept - C exception and cleanup mechanism.

   Copyright (C) 2012-2013 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <cexcept/libcexcept.h>
#include "libcexcept-private.h"

/**
 * SECTION:libcexcept
 * @short_description: libcexcept context
 *
 * The context contains the default values for the library user,
 * and is passed to all library operations.
 */

/**
 * cexcept_ctx:
 *
 * Opaque object representing the library context.
 */
struct cexcept_ctx {
        int refcount;
        void (*log_fn)(struct cexcept_ctx *ctx,
                       int priority, const char *file, int line, const char *fn,
                       const char *format, va_list args);
        void *userdata;
        int log_priority;
};

void cexcept_log(struct cexcept_ctx *ctx,
           int priority, const char *file, int line, const char *fn,
           const char *format, ...)
{
        va_list args;

        va_start(args, format);
        ctx->log_fn(ctx, priority, file, line, fn, format, args);
        va_end(args);
}

static void log_stderr(struct cexcept_ctx *ctx,
                       int priority, const char *file, int line, const char *fn,
                       const char *format, va_list args)
{
        fprintf(stderr, "libcexcept: %s: ", fn);
        vfprintf(stderr, format, args);
}

/**
 * cexcept_get_userdata:
 * @ctx: cexcept library context
 *
 * Retrieve stored data pointer from library context. This might be useful
 * to access from callbacks like a custom logging function.
 *
 * Returns: stored userdata
 **/
CEXCEPT_EXPORT void *cexcept_get_userdata(struct cexcept_ctx *ctx)
{
        if (ctx == NULL)
                return NULL;
        return ctx->userdata;
}

/**
 * cexcept_set_userdata:
 * @ctx: cexcept library context
 * @userdata: data pointer
 *
 * Store custom @userdata in the library context.
 **/
CEXCEPT_EXPORT void cexcept_set_userdata(struct cexcept_ctx *ctx, void *userdata)
{
        if (ctx == NULL)
                return;
        ctx->userdata = userdata;
}

static int log_priority(const char *priority)
{
        char *endptr;
        int prio;

        prio = strtol(priority, &endptr, 10);
        if (endptr[0] == '\0' || isspace(endptr[0]))
                return prio;
        if (strncmp(priority, "err", 3) == 0)
                return LOG_ERR;
        if (strncmp(priority, "info", 4) == 0)
                return LOG_INFO;
        if (strncmp(priority, "debug", 5) == 0)
                return LOG_DEBUG;
        return 0;
}

/**
 * cexcept_new:
 *
 * Create cexcept library context. This reads the cexcept configuration
 * and fills in the default values.
 *
 * The initial refcount is 1, and needs to be decremented to
 * release the resources of the cexcept library context.
 *
 * Returns: a new cexcept library context
 **/
CEXCEPT_EXPORT int cexcept_new(struct cexcept_ctx **ctx)
{
        const char *env;
        struct cexcept_ctx *c;

        c = calloc(1, sizeof(struct cexcept_ctx));
        if (!c)
                return -ENOMEM;

        c->refcount = 1;
        c->log_fn = log_stderr;
        c->log_priority = LOG_ERR;

        /* environment overwrites config */
        env = getenv("CEXCEPT_LOG");
        if (env != NULL)
                cexcept_set_log_priority(c, log_priority(env));

        info(c, "ctx %p created\n", c);
        dbg(c, "log_priority=%d\n", c->log_priority);
        *ctx = c;
        return 0;
}

/**
 * cexcept_ref:
 * @ctx: cexcept library context
 *
 * Take a reference of the cexcept library context.
 *
 * Returns: the passed cexcept library context
 **/
CEXCEPT_EXPORT struct cexcept_ctx *cexcept_ref(struct cexcept_ctx *ctx)
{
        if (ctx == NULL)
                return NULL;
        ctx->refcount++;
        return ctx;
}

/**
 * cexcept_unref:
 * @ctx: cexcept library context
 *
 * Drop a reference of the cexcept library context. If the refcount
 * reaches zero, the resources of the context will be released.
 *
 **/
CEXCEPT_EXPORT struct cexcept_ctx *cexcept_unref(struct cexcept_ctx *ctx)
{
        if (ctx == NULL)
                return NULL;
        ctx->refcount--;
        if (ctx->refcount > 0)
                return ctx;
        info(ctx, "context %p released\n", ctx);
        free(ctx);
        return NULL;
}

/**
 * cexcept_set_log_fn:
 * @ctx: cexcept library context
 * @log_fn: function to be called for logging messages
 *
 * The built-in logging writes to stderr. It can be
 * overridden by a custom function, to plug log messages
 * into the user's logging functionality.
 *
 **/
CEXCEPT_EXPORT void cexcept_set_log_fn(struct cexcept_ctx *ctx,
                              void (*log_fn)(struct cexcept_ctx *ctx,
                                             int priority, const char *file,
                                             int line, const char *fn,
                                             const char *format, va_list args))
{
        ctx->log_fn = log_fn;
        info(ctx, "custom logging function %p registered\n", log_fn);
}

/**
 * cexcept_get_log_priority:
 * @ctx: cexcept library context
 *
 * Returns: the current logging priority
 **/
CEXCEPT_EXPORT int cexcept_get_log_priority(struct cexcept_ctx *ctx)
{
        return ctx->log_priority;
}

/**
 * cexcept_set_log_priority:
 * @ctx: cexcept library context
 * @priority: the new logging priority
 *
 * Set the current logging priority. The value controls which messages
 * are logged.
 **/
CEXCEPT_EXPORT void cexcept_set_log_priority(struct cexcept_ctx *ctx, int priority)
{
        ctx->log_priority = priority;
}

struct cexcept_list_entry;
struct cexcept_list_entry *cexcept_list_entry_get_next(struct cexcept_list_entry *list_entry);
const char *cexcept_list_entry_get_name(struct cexcept_list_entry *list_entry);
const char *cexcept_list_entry_get_value(struct cexcept_list_entry *list_entry);

struct cexcept_thing {
        struct cexcept_ctx *ctx;
        int refcount;
};

CEXCEPT_EXPORT struct cexcept_thing *cexcept_thing_ref(struct cexcept_thing *thing)
{
        if (!thing)
                return NULL;
        thing->refcount++;
        return thing;
}

CEXCEPT_EXPORT struct cexcept_thing *cexcept_thing_unref(struct cexcept_thing *thing)
{
        if (thing == NULL)
                return NULL;
        thing->refcount--;
        if (thing->refcount > 0)
                return thing;
        dbg(thing->ctx, "context %p released\n", thing);
        free(thing);
        return NULL;
}

CEXCEPT_EXPORT struct cexcept_ctx *cexcept_thing_get_ctx(struct cexcept_thing *thing)
{
        return thing->ctx;
}

CEXCEPT_EXPORT int cexcept_thing_new_from_string(struct cexcept_ctx *ctx, const char *string, struct cexcept_thing **thing)
{
        struct cexcept_thing *t;

        t = calloc(1, sizeof(struct cexcept_thing));
        if (!t)
                return -ENOMEM;

        t->refcount = 1;
        t->ctx = ctx;
        *thing = t;
        return 0;
}

CEXCEPT_EXPORT struct cexcept_list_entry *cexcept_thing_get_some_list_entry(struct cexcept_thing *thing)
{
        return NULL;
}
