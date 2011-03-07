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

#include <stdio.h>
#include <glib.h>

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
      else if (ifile.event() == InFile::BLOB)
        {
          printf ("%s", spaces (indent).c_str());
          printf ("blob %s {\n", ifile.event_name().c_str());

          GenericIn *blob_in = ifile.open_blob();
          display_file (blob_in, indent + 2);

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
  if (argc != 2)
    {
      printf ("usage: %s <hexdata>\n", argv[0]);
      return 1;
    }
  vector<unsigned char> data;
  if (!HexString::decode (argv[1], data))
    {
      printf ("error decoding string\n");
      return 1;
    }

  int indent = 0;
  GenericIn *in = MMapIn::open_mem (&data[0], &data[data.size()]);
  display_file (in);
  delete in;
}
