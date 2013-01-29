/* Cleanups.
   Copyright (C) 1986, 1988-2005, 2007-2012 Free Software Foundation, Inc.

   This file is part of GDB.

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

#ifndef CLEANUPS_H
#define CLEANUPS_H

/* Outside of cleanups.c, this is an opaque type.  */
struct cexcept_cleanup;

/* NOTE: cagney/2000-03-04: This typedef is strictly for the
   make_cleanup function declarations below.  Do not use this typedef
   as a cast when passing functions into the make_cleanup() code.
   Instead either use a bounce function or add a wrapper function.
   Calling a f(char*) function with f(void*) is non-portable.  */
typedef void (cexcept_make_cleanup_ftype) (void *);

/* Function type for the dtor in make_cleanup_dtor.  */
typedef void (cexcept_make_cleanup_dtor_ftype) (void *);

/* WARNING: The result of the "make cleanup" routines is not the intuitive
   choice of being a handle on the just-created cleanup.  Instead it is an
   opaque handle of the cleanup mechanism and represents all cleanups created
   from that point onwards.
   The result is guaranteed to be non-NULL though.  */

extern struct cexcept_cleanup *cexcept_make_cleanup (cexcept_make_cleanup_ftype *, void *);

extern struct cexcept_cleanup *cexcept_make_cleanup_dtor (cexcept_make_cleanup_ftype *, void *,
					  cexcept_make_cleanup_dtor_ftype *);

extern struct cexcept_cleanup *cexcept_make_final_cleanup (cexcept_make_cleanup_ftype *, void *);

/* A special value to pass to do_cleanups and do_final_cleanups
   to tell them to do all cleanups.  */
extern struct cexcept_cleanup *cexcept_all_cleanups (void);

extern void cexcept_do_cleanups (struct cexcept_cleanup *);
extern void cexcept_do_final_cleanups (struct cexcept_cleanup *);

extern void cexcept_discard_cleanups (struct cexcept_cleanup *);
extern void cexcept_discard_final_cleanups (struct cexcept_cleanup *);

extern struct cexcept_cleanup *cexcept_save_cleanups (void);
extern struct cexcept_cleanup *cexcept_save_final_cleanups (void);

extern void cexcept_restore_cleanups (struct cexcept_cleanup *);
extern void cexcept_restore_final_cleanups (struct cexcept_cleanup *);

/* A no-op cleanup.
   This is useful when you want to establish a known reference point
   to pass to do_cleanups.  */
extern void cexcept_null_cleanup (void *);

#endif /* CLEANUPS_H */
