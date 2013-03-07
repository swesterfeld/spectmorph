// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smstdioin.hh"
#include "smstdiosubin.hh"
#include "smleakdebugger.hh"

#include <stdio.h>
#include <assert.h>

using namespace SpectMorph;

using std::string;

static LeakDebugger leak_debugger ("SpectMorph::StdioIn");

GenericIn*
StdioIn::open (const string& filename)
{
  FILE *file = fopen (filename.c_str(), "r");

  if (file)
    return new StdioIn (file, filename);
  else
    return NULL;
}

StdioIn::StdioIn (FILE *file, const string& filename) :
  file (file),
  filename (filename)
{
  leak_debugger.add (this);
}

StdioIn::~StdioIn()
{
  assert (file);
  fclose (file);
  leak_debugger.del (this);
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

unsigned char*
StdioIn::mmap_mem (size_t& remaining)
{
  return NULL;
}

size_t
StdioIn::get_pos()
{
  return ftell (file);
}

GenericIn *
StdioIn::open_subfile (size_t pos, size_t len)
{
  return StdioSubIn::open (filename, pos, len);
}
