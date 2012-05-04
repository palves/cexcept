/* Exception (throw catch) mechanism, for GDB, the GNU debugger.

   Copyright (C) 1986, 1988-2012 Free Software Foundation, Inc.

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

#ifndef CEXCEPT_H
#define CEXCEPT_H

#include <setjmp.h>
#include <stdarg.h>

/* FIXME */
#define ATTRIBUTE_NORETURN
#define ATTRIBUTE_PRINTF(a, b)

/* Reasons for calling throw_exception.  NOTE: all reason values must
   be less than zero.  enum value 0 is reserved for internal use as
   the return value from an initial setjmp.  The function
   catch_exceptions reserves values >= 0 as valid results from its
   wrapped function.  */

enum cexcept_return_reason
  {
    /* User interrupt.  */
    RETURN_QUIT = -2,
    /* Any other error.  */
    RETURN_ERROR
  };

#define RETURN_MASK(reason)	(1 << (int)(-reason))
#define RETURN_MASK_QUIT	RETURN_MASK (RETURN_QUIT)
#define RETURN_MASK_ERROR	RETURN_MASK (RETURN_ERROR)
#define RETURN_MASK_ALL		(RETURN_MASK_QUIT | RETURN_MASK_ERROR)
typedef int return_mask;

/* Describe all exceptions.  */

#define CEXCEPT_NO_ERROR 0

struct cexception
{
  enum cexcept_return_reason reason;
  int error;
  const char *message;
};

/* Wrap set/long jmp so that it's more portable (internal to
   exceptions).  */

#ifndef _WIN32
#define CEXCEPT_SIGJMP_BUF sigjmp_buf
#define CEXCEPT_SIGSETJMP(buf) sigsetjmp ((buf), 1)
#define CEXCEPT_SIGLONGJMP(buf, val) siglongjmp ((buf), (val))
#else
#define CEXCEPT_SIGJMP_BUF jmp_buf
#define CEXCEPT_SIGSETJMP(buf) setjmp (buf)
#define CEXCEPT_SIGLONGJMP(buf, val) longjmp ((buf), (val))
#endif

/* Functions to drive the exceptions state m/c (internal to
   exceptions).  */
CEXCEPT_SIGJMP_BUF *cexcept_state_mc_init
  (volatile struct cexception *exception,
   return_mask mask);
int cexcept_state_mc_action_iter (void);
int cexcept_state_mc_action_iter_1 (void);

/* Macro to wrap up standard try/catch behavior.

   The double loop lets us correctly handle code "break"ing out of the
   try catch block.  (It works as the "break" only exits the inner
   "while" loop, the outer for loop detects this handling it
   correctly.)  Of course "return" and "goto" are not so lucky.

   For instance:

   *INDENT-OFF*

   volatile struct cexception e;
   TRY_CATCH (e, RETURN_MASK_ERROR)
     {
     }
   switch (e.reason)
     {
     case RETURN_ERROR: ...
     }

  */

#define TRY_CATCH(EXCEPTION,MASK) \
     { \
       CEXCEPT_SIGJMP_BUF *buf = \
	 cexcept_state_mc_init (&(EXCEPTION), (MASK)); \
       CEXCEPT_SIGSETJMP (*buf); \
     } \
     while (cexcept_state_mc_action_iter ()) \
       while (cexcept_state_mc_action_iter_1 ())

/* *INDENT-ON* */

/* Throw an exception (as described by "struct cexception").  Will
   execute a LONG JUMP to the inner most containing exception handler
   established using catch_exceptions() (or similar).

   Code normally throws an exception using error() et.al.  For various
   reaons, GDB also contains code that throws an exception directly.
   For instance, the remote*.c targets contain CNTRL-C signal handlers
   that propogate the QUIT event up the exception chain.  ``This could
   be a good thing or a dangerous thing.'' -- the Existential
   Wombat.  */

extern void cexcept_throw (struct cexception exception)
     ATTRIBUTE_NORETURN;
extern void cexcept_throw_verror (int error, const char *fmt, va_list ap)
     ATTRIBUTE_NORETURN ATTRIBUTE_PRINTF (2, 0);
extern void cexcept_throw_vfatal (const char *fmt, va_list ap)
     ATTRIBUTE_NORETURN ATTRIBUTE_PRINTF (1, 0);
extern void cexcept_throw_error (int error, const char *fmt, ...)
     ATTRIBUTE_NORETURN ATTRIBUTE_PRINTF (2, 3);

#endif
