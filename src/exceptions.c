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

#include "config.h"

#include "exceptions.h"
#include "cleanups.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define internal_error(STR) \
  assert (0)

const struct cexception exception_none = { 0, CEXCEPT_NO_ERROR, NULL };

/* Possible catcher states.  */
enum catcher_state {
  /* Initial state, a new catcher has just been created.  */
  CATCHER_CREATED,
  /* The catch code is running.  */
  CATCHER_RUNNING,
  CATCHER_RUNNING_1,
  /* The catch code threw an exception.  */
  CATCHER_ABORTING
};

/* Possible catcher actions.  */
enum catcher_action {
  CATCH_ITER,
  CATCH_ITER_1,
  CATCH_THROWING
};

struct catcher
{
  enum catcher_state state;
  /* Jump buffer pointing back at the exception handler.  */
  CEXCEPT_SIGJMP_BUF buf;
  /* Status buffer belonging to the exception handler.  */
  volatile struct cexception *exception;
  /* Saved/current state.  */
  int mask;
  struct cleanup *saved_cleanup_chain;
  /* Back link.  */
  struct catcher *prev;
};

/* Where to go for throw_exception().  */
static struct catcher *current_catcher;

/* Return length of current_catcher list.  */

static int
catcher_list_size (void)
{
  int size;
  struct catcher *catcher;

  for (size = 0, catcher = current_catcher;
       catcher != NULL;
       catcher = catcher->prev)
    ++size;

  return size;
}

#define XZALLOC(TYPE) ((TYPE *) calloc (1, sizeof (TYPE)))

CEXCEPT_SIGJMP_BUF *
cexcept_state_mc_init (volatile struct cexception *exception,
		       return_mask mask)
{
  struct catcher *new_catcher = XZALLOC (struct catcher);

  /* Start with no exception, save it's address.  */
  exception->reason = 0;
  exception->error = CEXCEPT_NO_ERROR;
  exception->message = NULL;
  new_catcher->exception = exception;

  new_catcher->mask = mask;

  /* Prevent error/quit during FUNC from calling cleanups established
     prior to here.  */
  new_catcher->saved_cleanup_chain = save_cleanups ();

  /* Push this new catcher on the top.  */
  new_catcher->prev = current_catcher;
  current_catcher = new_catcher;
  new_catcher->state = CATCHER_CREATED;

  return &new_catcher->buf;
}

static void
catcher_pop (void)
{
  struct catcher *old_catcher = current_catcher;

  current_catcher = old_catcher->prev;

  /* Restore the cleanup chain, the error/quit messages, and the uiout
     builder, to their original states.  */

  restore_cleanups (old_catcher->saved_cleanup_chain);

  free (old_catcher);
}

/* Catcher state machine.  Returns non-zero if the m/c should be run
   again, zero if it should abort.  */

static int
cexcept_state_mc (enum catcher_action action)
{
  switch (current_catcher->state)
    {
    case CATCHER_CREATED:
      switch (action)
	{
	case CATCH_ITER:
	  /* Allow the code to run the catcher.  */
	  current_catcher->state = CATCHER_RUNNING;
	  return 1;
	default:
	  internal_error ("bad state");
	}
    case CATCHER_RUNNING:
      switch (action)
	{
	case CATCH_ITER:
	  /* No error/quit has occured.  Just clean up.  */
	  catcher_pop ();
	  return 0;
	case CATCH_ITER_1:
	  current_catcher->state = CATCHER_RUNNING_1;
	  return 1;
	case CATCH_THROWING:
	  current_catcher->state = CATCHER_ABORTING;
	  /* See also throw_exception.  */
	  return 1;
	default:
	  internal_error ("bad switch");
	}
    case CATCHER_RUNNING_1:
      switch (action)
	{
	case CATCH_ITER:
	  /* The did a "break" from the inner while loop.  */
	  catcher_pop ();
	  return 0;
	case CATCH_ITER_1:
	  current_catcher->state = CATCHER_RUNNING;
	  return 0;
	case CATCH_THROWING:
	  current_catcher->state = CATCHER_ABORTING;
	  /* See also throw_exception.  */
	  return 1;
	default:
	  internal_error ("bad switch");
	}
    case CATCHER_ABORTING:
      switch (action)
	{
	case CATCH_ITER:
	  {
	    struct cexception exception = *current_catcher->exception;

	    if (current_catcher->mask & RETURN_MASK (exception.reason))
	      {
		/* Exit normally if this catcher can handle this
		   exception.  The caller analyses the func return
		   values.  */
		catcher_pop ();
		return 0;
	      }
	    /* The caller didn't request that the event be caught,
	       relay the event to the next containing
	       catch_errors().  */
	    catcher_pop ();
	    cexcept_throw (exception);
	  }
	default:
	  internal_error ("bad state");
	}
    default:
      internal_error ("bad switch");
    }
}

int
cexcept_state_mc_action_iter (void)
{
  return cexcept_state_mc (CATCH_ITER);
}

int
cexcept_state_mc_action_iter_1 (void)
{
  return cexcept_state_mc (CATCH_ITER_1);
}

/* Return EXCEPTION to the nearest containing TRY_CATCH.  */

void
cexcept_throw (struct cexception exception)
{
  do_cleanups (all_cleanups ());

  /* Jump to the containing catch_errors() call, communicating REASON
     to that call via setjmp's return value.  Note that REASON can't
     be zero, by definition in defs.h.  */
  cexcept_state_mc (CATCH_THROWING);
  *current_catcher->exception = exception;
  CEXCEPT_SIGLONGJMP (current_catcher->buf, exception.reason);
}

/* A stack of exception messages.
   This is needed to handle nested calls to throw_it: we don't want to
   free space for a message before it's used.
   This can happen if we throw an exception during a cleanup:
   An outer TRY_CATCH may have an exception message it wants to print,
   but while doing cleanups further calls to throw_it are made.

   This is indexed by the size of the current_catcher list.
   It is a dynamically allocated array so that we don't care how deeply
   GDB nests its TRY_CATCHs.  */
static char **exception_messages;

/* The number of currently allocated entries in exception_messages.  */
static int exception_messages_size;

static void ATTRIBUTE_NORETURN ATTRIBUTE_PRINTF (3, 0)
throw_it (enum cexcept_return_reason reason, int error, const char *fmt,
	  va_list ap)
{
  struct cexception e;
  char *new_message;
  int depth = catcher_list_size ();

  assert (depth > 0);

  /* Note: The new message may use an old message's text.  */
  vasprintf (&new_message, fmt, ap);

  if (depth > exception_messages_size)
    {
      int old_size = exception_messages_size;

      exception_messages_size = depth + 10;
      exception_messages = (char **) realloc (exception_messages,
					      exception_messages_size
					      * sizeof (char *));
      memset (exception_messages + old_size, 0,
	      (exception_messages_size - old_size) * sizeof (char *));
    }

  free (exception_messages[depth - 1]);
  exception_messages[depth - 1] = new_message;

  /* Create the exception.  */
  e.reason = reason;
  e.error = error;
  e.message = new_message;

  /* Throw the exception.  */
  cexcept_throw (e);
}

void
cexcept_throw_verror (int error, const char *fmt, va_list ap)
{
  throw_it (RETURN_ERROR, error, fmt, ap);
}

void
cexcept_throw_vfatal (const char *fmt, va_list ap)
{
  throw_it (RETURN_QUIT, CEXCEPT_NO_ERROR, fmt, ap);
}

void
cexcept_throw_error (int error, const char *fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  throw_it (RETURN_ERROR, error, fmt, args);
  va_end (args);
}
