// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smstdioin.hh"
#include "smstdiosubin.hh"

#include <stdio.h>
#include <assert.h>

using namespace SpectMorph;

using std::string;

GenericInP
StdioIn::open (const string& filename)
{
  FILE *file = fopen (filename.c_str(), "rb");

  if (file)
    return GenericInP (new StdioIn (file, filename));
  else
    return NULL;
}

StdioIn::StdioIn (FILE *file, const string& filename) :
  file (file),
  filename (filename)
{
}

StdioIn::~StdioIn()
{
  assert (file);
  fclose (file);
}

int
StdioIn::get_byte()
{
  return fgetc (file);
}

int
StdioIn::read (void *ptr, size_t size)
{
  return fread (ptr, 1, size, file);
}

bool
StdioIn::skip (size_t size)
{
  if (fseek (file, size, SEEK_CUR) == 0)
    return true;
  return false;
}

const unsigned char*
StdioIn::mmap_mem (size_t& remaining)
{
  return NULL;
}

size_t
StdioIn::get_pos()
{
  return ftell (file);
}

GenericInP
StdioIn::open_subfile (size_t pos, size_t len)
{
  return StdioSubIn::open (filename, pos, len);
}
