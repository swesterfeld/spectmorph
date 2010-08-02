/*
 * Copyright (C) 2010 Stefan Westerfeld
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

#include "smoutfile.hh"
#include "sminfile.hh"
#include "smgenericin.hh"

#include <glib.h>

#include <stdlib.h>
#include <assert.h>

using SpectMorph::InFile;
using SpectMorph::OutFile;
using SpectMorph::GenericIn;
using std::vector;
using std::string;

vector<unsigned char> xdata;

void
create_testblob()
{
  OutFile outfile ("testblob.out", "SpectMorph::TestBlob");

  for (int i = 0; i < 1024; i++)
    xdata.push_back (g_random_int());

  outfile.begin_section ("header");
  outfile.write_string ("name", "BLOB Test");
  outfile.write_blob ("blob", &xdata[0], xdata.size());
  outfile.end_section();
}

void
read_testblob()
{
  InFile infile ("testblob.out");
  string contents;

  if (!infile.open_ok())
    {
      fprintf (stderr, "can't load testblob.out\n");
      exit (1);
    }
  while (infile.event() != InFile::END_OF_FILE)
    {
      if (infile.event() == InFile::BEGIN_SECTION)
        {
          contents += "<section " + infile.event_name() + ">";
        }
      else if (infile.event() == InFile::END_SECTION)
        {
          contents += "</section>";
        }
      else if (infile.event() == InFile::STRING)
        {
          contents += "<string>" + infile.event_data() + "</string>";
        }
      else if (infile.event() == InFile::BLOB)
        {
          contents += "<blob>";
          GenericIn *in_blob = infile.open_blob();
          assert (in_blob);
          for (int i = 0; i < xdata.size(); i++)
            {
              assert (in_blob->get_byte() == xdata[i]);
            }
          assert (in_blob->get_byte() == EOF);
        }
      else
        {
          assert (false);
        }
      infile.next_event();
    }
  printf ("testblob: %s\n", contents.c_str());
  if (contents != "<section header><string>BLOB Test</string><blob></section>")
    {
      printf ("testblob: bad contents\n");
      exit (1);
    }
}

int
main()
{
  create_testblob();
  read_testblob();

  if (unlink ("testblob.out") != 0)
    {
      perror ("unlink testblob.out failed");
      return 1;
    }
}
