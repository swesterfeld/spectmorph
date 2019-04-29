// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdio.h>
#include <string.h>

#include "smzip.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

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
          printf ("ERR=%d\n", reader.error());
        }
    }
  if (argc == 3 && strcmp (argv[1], "get") == 0)
    {
      ZipReader reader (argv[2]);

      for (auto name : reader.filenames())
        {
          auto data = reader.read (name);

          printf ("%s [[[\n", name.c_str());
          for (auto ch : data)
            printf ("%c", ch);
          printf ("]]]\n\n");
        }
    }
  if (argc == 3 && strcmp (argv[1], "create") == 0)
    {
      ZipWriter writer (argv[2]);

      writer.add ("test.txt", "Hello World!\n");
      writer.add ("test2.txt", "Test II\n");

      if (writer.error())
        printf ("ERR=%d\n", writer.error());
    }
  if (argc == 3 && strcmp (argv[1], "create-mem") == 0)
    {
      ZipWriter writer (argv[2]);

      writer.add ("test.txt", "Hello World!\n");
      writer.add ("test2.txt", "Test II\n");

      if (writer.error())
        printf ("ERR=%d\n", writer.error());

      FILE *f = fopen (argv[2], "w");
      for (auto c : writer.data())
        fputc (c, f);
      fclose (f);
    }
}
