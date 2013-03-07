// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmmapin.hh"
#include "smleakdebugger.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
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
    return NULL;

  int fd = ::open (filename.c_str(), O_RDONLY);
  if (fd >= 0)
    {
      struct stat st;

      if (fstat (fd, &st) == 0)
        {
          unsigned char *mapfile = static_cast<unsigned char *> (mmap (NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
          if (mapfile != MAP_FAILED)
            {
              return new MMapIn (mapfile, mapfile + st.st_size, fd);
            }
        }
    }
  return NULL;
}

GenericIn*
MMapIn::open_mem (unsigned char *begin, unsigned char *end)
{
  return new MMapIn (begin, end, -1);
}

MMapIn::MMapIn (unsigned char *mapfile, unsigned char *mapend, int fd) :
  mapfile (mapfile),
  mapend (mapend),
  fd (fd)
{
  pos = static_cast<unsigned char *> (mapfile);

  leak_debugger.add (this);
}

MMapIn::~MMapIn()
{
  if (fd >= 0)
    {
      munmap (mapfile, mapend - mapfile);
      close (fd);
    }
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
  return new MMapIn (mapfile + pos, mapfile + pos + len, -1 /* no fd */);
}
