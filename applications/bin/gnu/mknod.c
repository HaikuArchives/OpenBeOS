/* mknod -- make special files
   Copyright (C) 90, 91, 1995-2000 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* Usage: mknod [-m mode] [--mode=mode] path {bcu} major minor
		make a block or character device node
          mknod [-m mode] [--mode=mode] path p
		make a FIFO (named pipe)

   Options:
   -m, --mode=mode	Set the mode of created nodes to MODE, which is
			symbolic as in chmod and uses the umask as a point of
			departure.

   David MacKenzie <djm@ai.mit.edu>  */

#include <config.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>

#include "system.h"
#include "error.h"
#include "modechange.h"
#include "quote.h"
#include "xstrtol.h"

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "mknod"

#define AUTHORS "David MacKenzie"

/* The name this program was run with. */
char *program_name;

static struct option const longopts[] =
{
  {"mode", required_argument, NULL, 'm'},
  {GETOPT_HELP_OPTION_DECL},
  {GETOPT_VERSION_OPTION_DECL},
  {NULL, 0, NULL, 0}
};

void
usage (int status)
{
  if (status != 0)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      printf (_("Usage: %s [OPTION]... NAME TYPE [MAJOR MINOR]\n"), program_name);
      printf (_("\
Create the special file NAME of the given TYPE.\n\
\n\
  -m, --mode=MODE   set permission mode (as in chmod), not a=rw - umask\n\
      --help        display this help and exit\n\
      --version     output version information and exit\n\
\n\
MAJOR MINOR are forbidden for TYPE p, mandatory otherwise.  TYPE may be:\n\
\n\
  b      create a block (buffered) special file\n\
  c, u   create a character (unbuffered) special file\n\
  p      create a FIFO\n\
"));
      puts (_("\nReport bugs to <bug-fileutils@gnu.org>."));
    }
  exit (status);
}

int
main (int argc, char **argv)
{
  mode_t newmode;
  struct mode_change *change;
  const char *specified_mode;
  int optc;
  int i_major, i_minor;
  long int tmp_major, tmp_minor;
  char *s;

  program_name = argv[0];
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  atexit (close_stdout);

  specified_mode = NULL;

  while ((optc = getopt_long (argc, argv, "m:", longopts, NULL)) != -1)
    {
      switch (optc)
	{
	case 0:
	  break;
	case 'm':
	  specified_mode = optarg;
	  break;
	case_GETOPT_HELP_CHAR;
	case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);
	default:
	  usage (1);
	}
    }

  newmode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (specified_mode)
    {
      newmode &= ~ umask (0);
      change = mode_compile (specified_mode, 0);
      if (change == MODE_INVALID)
	error (1, 0, _("invalid mode"));
      else if (change == MODE_MEMORY_EXHAUSTED)
	xalloc_die ();
      newmode = mode_adjust (newmode, change);
    }

  if (argc - optind != 2 && argc - optind != 4)
    {
      const char *msg;
      if (argc - optind < 2)
	msg = _("too few arguments");
      else if (argc - optind > 4)
	msg = _("too many arguments");
      else
	msg = _("wrong number of arguments");
      error (0, 0, msg);
      usage (1);
    }

  /* Only check the first character, to allow mnemonic usage like
     `mknod /dev/rst0 character 18 0'. */

  switch (argv[optind + 1][0])
    {
    case 'b':			/* `block' or `buffered' */
#ifndef S_IFBLK
      error (4, 0, _("block special files not supported"));
#else
      if (argc - optind != 4)
	{
	  error (0, 0, _("\
when creating block special files, major and minor device\n\
numbers must be specified"));
	  usage (1);
	}

      s = argv[optind + 2];
      if (xstrtol (s, NULL, 0, &tmp_major, NULL) != LONGINT_OK)
	error (1, 0, _("invalid major device number %s"), quote (s));

      s = argv[optind + 3];
      if (xstrtol (s, NULL, 0, &tmp_minor, NULL) != LONGINT_OK)
	error (1, 0, _("invalid minor device number %s"), quote (s));

      i_major = (int) tmp_major;
      i_minor = (int) tmp_minor;

      if (mknod (argv[optind], newmode | S_IFBLK, makedev (i_major, i_minor)))
	error (1, errno, "%s", quote (argv[optind]));
#endif
      break;

    case 'c':			/* `character' */
    case 'u':			/* `unbuffered' */
#ifndef S_IFCHR
      error (4, 0, _("character special files not supported"));
#else
      if (argc - optind != 4)
	{
	  error (0, 0, _("\
when creating character special files, major and minor device\n\
numbers must be specified"));
	  usage (1);
	}

      s = argv[optind + 2];
      if (xstrtol (s, NULL, 0, &tmp_major, NULL) != LONGINT_OK)
	error (1, 0, _("invalid major device number %s"), quote (s));

      s = argv[optind + 3];
      if (xstrtol (s, NULL, 0, &tmp_minor, NULL) != LONGINT_OK)
	error (1, 0, _("invalid minor device number %s"), quote (s));

      i_major = (int) tmp_major;
      i_minor = (int) tmp_minor;

      if (mknod (argv[optind], newmode | S_IFCHR, makedev (i_major, i_minor)))
	error (1, errno, "%s", quote (argv[optind]));
#endif
      break;

    case 'p':			/* `pipe' */
#ifndef S_ISFIFO
      error (4, 0, _("fifo files not supported"));
#else
      if (argc - optind != 2)
	{
	  error (0, 0, _("\
major and minor device numbers may not be specified for fifo files"));
	  usage (1);
	}
      if (mkfifo (argv[optind], newmode))
	error (1, errno, "%s", quote (argv[optind]));
#endif
      break;

    default:
      usage (1);
    }

  /* Perform an explicit chmod to ensure the file mode permission bits
     are set as specified.  This extra step is necessary in some cases
     when the containing directory has a default ACL.  */

  if (specified_mode)
    {
      if (chmod (argv[optind], newmode))
        error (0, errno, _("cannot set permissions of `%s'"),
               quote (argv[optind]));
    }

  exit (0);
}
