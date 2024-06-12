// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smstdioout.hh"
#include <stdio.h>

using namespace SpectMorph;

GenericOutP
StdioOut::open (const std::string& filename)
{
  FILE *file = fopen (filename.c_str(), "wb");

  if (file)
    return GenericOutP (new StdioOut (file));
  else
    return nullptr;
}

StdioOut::StdioOut (FILE *file)
  : file (file)
{
}

StdioOut::~StdioOut()
{
  if (file != NULL)
    {
      fclose (file);
      file = NULL;
    }
}

int
StdioOut::put_byte (int c)
{
  return fputc (c, file);
}

int
StdioOut::write (const void *ptr, size_t size)
{
  if (size == 0)
    return 0;

  return fwrite (ptr, 1, size, file);
}
