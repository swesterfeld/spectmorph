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
#include "smmemout.hh"
#include "smmain.hh"

#include <map>

#include <glib.h>
#include <stdlib.h>

#define PROG_NAME "smfileedit"

using namespace SpectMorph;

using std::vector;
using std::string;
using std::map;

void
process_file (InFile& ifile, OutFile& ofile)
{
  map<string, vector<unsigned char> > output_blob_map;

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
          GenericIn *blob_in = ifile.open_blob();
          if (!blob_in)
            {
              fprintf (stderr, PROG_NAME ": error opening input\n");
              exit (1);
            }
          InFile blob_ifile (blob_in);

          vector<unsigned char> blob_data;
          MemOut                blob_mo (&blob_data);

          {
            OutFile blob_ofile (&blob_mo, blob_ifile.file_type(), blob_ifile.file_version());
            process_file (blob_ifile, blob_ofile);
            // blob_ofile destructor run here
          }

          ofile.write_blob (ifile.event_name(), &blob_data[0], blob_data.size());
          delete blob_in;

          output_blob_map[ifile.event_blob_sum()] = blob_data;
        }
      else if (ifile.event() == InFile::BLOB_REF)
        {
          vector<unsigned char>& blob_data = output_blob_map[ifile.event_blob_sum()];

          ofile.write_blob (ifile.event_name(), &blob_data[0], blob_data.size());
        }
      else if (ifile.event() == InFile::READ_ERROR)
        {
          g_printerr (PROG_NAME ": read error\n");
          break;
        }
      else
        {
          g_printerr (PROG_NAME ": unhandled event %d\n", ifile.event());
        }
      ifile.next_event();
    }
}

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

  process_file (ifile, ofile);
  delete in;
}
