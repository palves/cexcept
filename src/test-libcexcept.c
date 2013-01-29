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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <cexcept/libcexcept.h>

/* Convenience aliases.  You'd do these project-wide.  */
#define TRY_CATCH CEXCEPT_TRY
#define throw_error cexcept_throw_error
#define make_cleanup cexcept_make_cleanup
#define cleanup cexcept_cleanup
#define do_cleanups cexcept_do_cleanups
#define discard_cleanups cexcept_discard_cleanups

/* Helper function which does the work for make_cleanup_fclose.  */

static void
do_fclose_cleanup (void *arg)
{
  FILE *file = arg;

  fclose (file);
}

/* Return a new cleanup that closes FILE.  */

static struct cexcept_cleanup *
make_cleanup_fclose (FILE *file)
{
  return make_cleanup (do_fclose_cleanup, file);
}

/* This function is useful for cleanups.
   Do

   foo = malloc (...);
   old_chain = make_cleanup (free_current_contents, &foo);

   to arrange to free the object thus allocated.  */

static void
free_current_contents (void *ptr)
{
  void **location = ptr;

  assert (location != NULL);

  if (*location != NULL)
    {
      free (*location);
      *location = NULL;
    }
}


static void
test_fclose (void)
{
  struct cleanup *old_chain;
  FILE *input;
  char buf[10];
  size_t bytes_read;

  input = fopen ("/dev/null", "r");
  old_chain = make_cleanup_fclose (input);

  /* use INPUT.  */
  bytes_read = fread (buf, 1, sizeof (buf), input);

  if (bytes_read == 0)
    throw_error (GENERIC_ERROR, "error: read some bytes\n");

  do_cleanups (old_chain);
}

static char *
test_realloc (int arg)
{
  struct cleanup *old_chain;
  char *ret;

  ret = malloc (10);
  old_chain = make_cleanup (free_current_contents, &ret);

  if (arg == 1)
    {
      /* Success.  */
      discard_cleanups (old_chain);
      return ret;
    }

  /* Make it bigger! */
  ret = realloc (ret, 100);

  if (arg == 2)
    {
      /* Success.  */
      discard_cleanups (old_chain);
      return ret;
    }

  /* Error return.  */
  do_cleanups (old_chain);
  return ret;
}

int
main (int argc, char *argv[])
{
  volatile struct cexception e;
  struct cleanup *old_chain;

  TRY_CATCH (e, RETURN_MASK_ERROR)
    {
      test_fclose ();
    }
  if (e.reason < 0)
    {
      fprintf (stderr, "caught: %s", e.message);
    }

  TRY_CATCH (e, RETURN_MASK_ERROR)
    {
      char *a;

      a = malloc (10);
      old_chain = make_cleanup (free, a);

      discard_cleanups (old_chain);

      free (a);
    }
  if (e.reason < 0)
    {
      fprintf (stderr, "caught: %s", e.message);
      return EXIT_SUCCESS;
    }

  TRY_CATCH (e, RETURN_MASK_ERROR)
    {
      char *a;

      a = test_realloc (1);
      free (a);
      a = test_realloc (2);
      free (a);
      a = test_realloc (3);
      free (a);
    }
  if (e.reason < 0)
    {
      fprintf (stderr, "caught: %s", e.message);
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
