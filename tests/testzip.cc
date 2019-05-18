// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdio.h>
#include <string.h>

#include "smzip.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

void
get (ZipReader& reader)
{
  for (auto name : reader.filenames())
    {
      auto data = reader.read (name);

      printf ("%s [[[\n", name.c_str());
      for (auto ch : data)
        printf ("%c", ch);
      printf ("]]]\n\n");
    }
}

void
create (ZipWriter& writer)
{
  writer.add ("test.txt", "Hello World!\n");
  writer.add ("test2.txt", "Test II\n");

  if (writer.error())
    printf ("ERR: %s\n", writer.error().message());
}

int
main (int argc, char **argv)
{
  if (argc == 3 && strcmp (argv[1], "list") == 0)
    {
      ZipReader reader (argv[2]);

      for (auto name : reader.filenames())
        printf ("%s\n", name.c_str());

      if (reader.error())
        {
          printf ("ERR: %s\n", reader.error().message());
        }
    }
  if (argc == 3 && strcmp (argv[1], "get") == 0)
    {
      ZipReader reader (argv[2]);

      get (reader);
    }
  if (argc == 3 && strcmp (argv[1], "create") == 0)
    {
      ZipWriter writer (argv[2]);

      create (writer);
    }
  if (argc == 3 && strcmp (argv[1], "create-mem") == 0)
    {
      ZipWriter writer;

      create (writer);

      FILE *f = fopen (argv[2], "w");
      for (auto c : writer.data())
        fputc (c, f);
      fclose (f);
    }
  if (argc == 3 && strcmp (argv[1], "get-mem") == 0)
    {
      vector<uint8_t> data;

      FILE *f = fopen (argv[2], "r");
      int c;
      while ((c = fgetc (f)) >= 0)
        data.push_back (c);
      fclose (f);

      ZipReader reader (data);

      get (reader);
    }
}
