// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smoutfile.hh"
#include "sminfile.hh"
#include "smgenericin.hh"

#include <glib.h>

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

using namespace SpectMorph;

using std::vector;
using std::string;

vector<unsigned char> xdata;

void
create_testblob()
{
  OutFile outfile ("testblob.out", "SpectMorph::TestBlob", 42);

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
          for (size_t i = 0; i < xdata.size(); i++)
            {
              assert (in_blob->get_byte() == xdata[i]);
            }
          assert (in_blob->get_byte() == EOF);
          delete in_blob; // close subfile handle
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
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  create_testblob();
  read_testblob();

  if (unlink ("testblob.out") != 0)
    {
      perror ("unlink testblob.out failed");
      return 1;
    }
}
