// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmmapin.hh"
#include "smleakdebugger.hh"
#include "smutils.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::MMapIn");

GenericIn*
MMapIn::open (const std::string& filename)
{
  if (getenv ("SPECTMORPH_NOMMAP"))
    return nullptr;

  GMappedFile *gmf = g_mapped_file_new (filename.c_str(), FALSE, nullptr);
  if (gmf)
    {
      unsigned char *bytes  = reinterpret_cast<unsigned char *> (g_mapped_file_get_contents (gmf));
      size_t         length = g_mapped_file_get_length (gmf);

      return new MMapIn (bytes, bytes + length, gmf);
    }
  else
    {
      return nullptr;
    }
}

GenericIn*
MMapIn::open_mem (unsigned char *begin, unsigned char *end)
{
  return new MMapIn (begin, end, nullptr);
}

MMapIn::MMapIn (unsigned char *mapfile, unsigned char *mapend, GMappedFile *gmf) :
  mapfile (mapfile),
  mapend (mapend),
  g_mapped_file (gmf)
{
  pos = static_cast<unsigned char *> (mapfile);

  leak_debugger.add (this);
}

MMapIn::~MMapIn()
{
  if (g_mapped_file)
    g_mapped_file_unref (g_mapped_file);

  leak_debugger.del (this);
}

int
MMapIn::get_byte()
{
  if (pos < mapend)
    return *pos++;
  else
    return EOF;
}

int
MMapIn::read (void *ptr, size_t size)
{
  if (pos + size <= mapend)
    {
      memcpy (ptr, pos, size);
      pos += size;
      return size;
    }
  else
    return 0;
}

bool
MMapIn::skip (size_t size)
{
  if (pos + size <= mapend)
    {
      pos += size;
      return true;
    }
  else
    {
      return false;
    }
}

unsigned char *
MMapIn::mmap_mem (size_t& remaining)
{
  remaining = mapend - pos;
  return pos;
}

size_t
MMapIn::get_pos()
{
  return pos - mapfile;
}

GenericIn *
MMapIn::open_subfile (size_t pos, size_t len)
{
  return new MMapIn (mapfile + pos, mapfile + pos + len, nullptr /* no mapped file */);
}
