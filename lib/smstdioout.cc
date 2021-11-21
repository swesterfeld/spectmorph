// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smstdioout.hh"
#include "smleakdebugger.hh"
#include <stdio.h>

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::StdioOut");

GenericOut*
StdioOut::open (const std::string& filename)
{
  FILE *file = fopen (filename.c_str(), "wb");

  if (file)
    return new StdioOut (file);
  else
    return NULL;
}

StdioOut::StdioOut (FILE *file)
  : file (file)
{
  leak_debugger.add (this);
}

StdioOut::~StdioOut()
{
  if (file != NULL)
    {
      fclose (file);
      file = NULL;
    }
  leak_debugger.del (this);
}

int
StdioOut::put_byte (int c)
{
  return fputc (c, file);
}

int
StdioOut::write (const void *ptr, size_t size)
{
  return fwrite (ptr, 1, size, file);
}
