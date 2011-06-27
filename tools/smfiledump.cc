/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smhexstring.hh"
#include "sminfile.hh"
#include "smmain.hh"
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

#define PROG_NAME "smfiledump"

using namespace SpectMorph;

using std::vector;
using std::string;

string
spaces (int indent)
{
  string s;
  for (int i = 0; i < indent; i++)
    s += " ";
  return s;
}

/// @cond
struct Options
{
  string	      program_name; /* FIXME: what to do with that */
  bool                full;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

Options::Options () :
  program_name ("smfiledump"),
  full (false)
{
}

void
Options::parse (int   *argc_p,
                char **argv_p[])
{
  guint argc = *argc_p;
  gchar **argv = *argv_p;
  unsigned int i, e;

  g_return_if_fail (argc >= 0);

  /*  I am tired of seeing .libs/lt-gst123 all the time,
   *  but basically this should be done (to allow renaming the binary):
   *
  if (argc && argv[0])
    program_name = argv[0];
  */

  for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "--help") == 0 ||
          strcmp (argv[i], "-h") == 0)
	{
	  print_usage();
	  exit (0);
	}
      else if (strcmp (argv[i], "--version") == 0 || strcmp (argv[i], "-v") == 0)
	{
	  printf ("%s %s\n", program_name.c_str(), VERSION);
	  exit (0);
	}
      else if (check_arg (argc, argv, &i, "--full") || check_arg (argc, argv, &i, "-f"))
        {
          full = true;
        }
    }

  /* resort argc/argv */
  e = 1;
  for (i = 1; i < argc; i++)
    if (argv[i])
      {
        argv[e++] = argv[i];
        if (i >= e)
          argv[i] = NULL;
      }
  *argc_p = e;
}

void
Options::print_usage ()
{
  printf ("usage: %s [<hexdata>|<filename>]\n", options.program_name.c_str());
  printf ("\n");
  printf ("options:\n");
  printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  printf (" -v, --version               print version\n");
  printf (" -f, --full                  dump float block data\n");
  printf ("\n");
}


static void
display_file (GenericIn *in, int indent = 0)
{
  InFile ifile (in);
  printf ("%sfile header {\n", spaces (indent).c_str());
  printf ("%s  type = %s\n", spaces (indent).c_str(), ifile.file_type().c_str());
  printf ("%s  version = %d\n", spaces (indent).c_str(), ifile.file_version());
  printf ("%s}\n\n", spaces (indent).c_str());
  while (ifile.event() != InFile::END_OF_FILE)
    {
     if (ifile.event() == InFile::BEGIN_SECTION)
        {
          printf ("%s", spaces (indent).c_str());
          printf ("section %s {\n", ifile.event_name().c_str());
          indent += 2;
        }
      else if (ifile.event() == InFile::END_SECTION)
        {
          indent -= 2;
          printf ("%s", spaces (indent).c_str());
          printf ("}\n");
        }
      else if (ifile.event() == InFile::STRING)
        {
          printf ("%s", spaces (indent).c_str());
          printf ("string %s = \"%s\"\n", ifile.event_name().c_str(), ifile.event_data().c_str());
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          printf ("%s", spaces (indent).c_str());
          printf ("float %s = %.7g\n", ifile.event_name().c_str(), ifile.event_float());
        }
      else if (ifile.event() == InFile::FLOAT_BLOCK)
        {
          printf ("%s", spaces (indent).c_str());
          if (options.full)
            {
              const vector<float>& fb = ifile.event_float_block();
              printf ("float_block %s[%zd] = {\n", ifile.event_name().c_str(), fb.size());
              for (size_t i = 0; i < fb.size(); i++)
                {
                  printf ("%s  %.17g%s\n", spaces (indent).c_str(), fb[i], (i + 1) == fb.size() ? "" : ",");
                }
              printf ("%s}\n", spaces (indent).c_str());
            }
          else
            {
              printf ("float_block %s[%zd] = {...}\n", ifile.event_name().c_str(), ifile.event_float_block().size());
            }
        }
      else if (ifile.event() == InFile::INT)
        {
          printf ("%s", spaces (indent).c_str());
          printf ("int %s = %d\n", ifile.event_name().c_str(), ifile.event_int());
        }
      else if (ifile.event() == InFile::BOOL)
        {
          printf ("%s", spaces (indent).c_str());
          printf ("bool %s = %s\n", ifile.event_name().c_str(), ifile.event_bool() ? "true" : "false");
        }
      else if (ifile.event() == InFile::BLOB)
        {
          printf ("%s", spaces (indent).c_str());
          printf ("blob %s {\n", ifile.event_name().c_str());

          GenericIn *blob_in = ifile.open_blob();
          if (!blob_in)
            {
              fprintf (stderr, PROG_NAME ": error opening input\n");
              exit (1);
            }
          display_file (blob_in, indent + 2);
          delete blob_in;

          printf ("%s}\n", spaces (indent).c_str());
        }
#if 0
      else if (ifile.event() == InFile::BLOB_REF)
        {
          if (section == "operator")
            {
              if (ifile.event_name() == "data")
                {
                  vector<unsigned char>& blob_data = blob_data_map[ifile.event_blob_sum()];

                  GenericIn *in = MMapIn::open_mem (&blob_data[0], &blob_data[blob_data.size()]);
                  InFile blob_infile (in);
                  load_op->load (blob_infile);

                  add_operator (load_op);
                }
            }
        }
#endif
      else if (ifile.event() == InFile::READ_ERROR)
        {
          g_printerr ("read error\n");
          break;
        }
      else
        {
          printf ("unhandled event %d\n", ifile.event());
        }
      ifile.next_event();
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  options.parse (&argc, &argv);

  if (argc != 2)
    {
      options.print_usage();
      return 1;
    }

  vector<unsigned char> data;

  GenericIn *in = StdioIn::open (argv[1]);
  if (!in)
    {
      if (!HexString::decode (argv[1], data))
        {
          fprintf (stderr, PROG_NAME ": error decoding string\n");
          return 1;
        }
      in = MMapIn::open_mem (&data[0], &data[data.size()]);
    }
  if (!in)
    {
      fprintf (stderr, PROG_NAME ": error opening input\n");
      return 1;
    }
  display_file (in);
  delete in;
}
