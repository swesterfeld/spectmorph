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

#include "sminfile.hh"
#include "smoutfile.hh"
#include "smmain.hh"

#define PROG_NAME "smfileedit"

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  if (argc != 3)
    {
      printf ("usage: " PROG_NAME " <infile> <outfile> [<command>...]\n");
      return 1;
    }
  GenericIn *in = StdioIn::open (argv[1]);
  if (!in)
    {
      fprintf (stderr, PROG_NAME ": error opening input\n");
      return 1;
    }
  InFile ifile (in);
  OutFile ofile (argv[2], ifile.file_type(), ifile.file_version());

  while (ifile.event() != InFile::END_OF_FILE)
    {
     if (ifile.event() == InFile::BEGIN_SECTION)
        {
          ofile.begin_section (ifile.event_name());
        }
      else if (ifile.event() == InFile::END_SECTION)
        {
          ofile.end_section();
        }
      else if (ifile.event() == InFile::STRING)
        {
          ofile.write_string (ifile.event_name(), ifile.event_data());
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          ofile.write_float (ifile.event_name(), ifile.event_float());
        }
      else if (ifile.event() == InFile::FLOAT_BLOCK)
        {
          ofile.write_float_block (ifile.event_name(), ifile.event_float_block());
        }
      else if (ifile.event() == InFile::INT)
        {
          ofile.write_int (ifile.event_name().c_str(), ifile.event_int());
        }
      else if (ifile.event() == InFile::BOOL)
        {
          ofile.write_bool (ifile.event_name(), ifile.event_bool());
        }
      else if (ifile.event() == InFile::BLOB)
        {
          printf ("<no blob support>\n");
        }
      ifile.next_event();
    }
  delete in;
}
